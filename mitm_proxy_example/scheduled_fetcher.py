import asyncio
import json
import os
from datetime import datetime
import time
from construct_goddess_req import query_quest_stats,query_guild_members
from update_users_db import merge_in_db_quest_status, merge_in_db_guild_members
from gen_users_html import gen_new_html
from gen_war_map_logs import reset_war_logs, gen_war_map_logs, get_war_map_logs
import subprocess

DAILY_JOBS = [
    {
        "run_time": "21:50",
        "days": ["tue", "wed", "thu", "fri", "sat", "sun"],
        "job_type": "merge_war"
    },
    {
        "run_time": "12:55",
        "days": ["sun"],
        "job_type": "merge_quests"
    },
    {
        "run_time": "22:00",
        "days": ["tue", "wed", "thu", "fri", "sat", "sun"],
        "job_type": "reset_logs"
    },
]

# spread out evenly in 1 hour to update the page regurarly
HOURLY_JOBS = [
    {
        "run_time": "5", # minute of the hour
        "days": ["mon", "tue", "wed", "thu", "fri", "sat", "sun"],
        "job_type": "temporary_war"
    },
    {
        "run_time": "35", # minute of the hour
        "days": ["mon", "tue", "wed", "thu", "fri", "sat", "sun"],
        "job_type": "temporary_quests"
    },
]

MINUTE_JOBS = [
    {
        "run_time": 2, # nth minute of the hour
        "days": ["mon", "tue", "wed", "thu", "fri", "sat", "sun"],
        "job_type": "war_logs"
    },
]

test_runs_to_make = 2
test_runs_made = 0

def should_run_daily_now(job, now):
    global test_runs_made, test_runs_to_make
    if test_runs_made < test_runs_to_make:
        test_runs_made += 1
        #return True
    now_str = now.strftime("%H:%M")
    today = now.strftime("%a").lower()
    return job["run_time"] == now_str and today in job["days"]

def should_run_hourly_now(job, now):
    now_minute = now.strftime("%M")
    today = now.strftime("%a").lower()
    return job["run_time"] == now_minute and today in job["days"]

def should_run_minute_now(job, now):
    now_minute = int(now.strftime("%M")) % int(job["run_time"])
    today = now.strftime("%a").lower()
    return now_minute == 0 and today in job["days"]
    
def deploy_firebase():
    try:
        result = subprocess.run(['firebase', 'deploy'], capture_output=True, text=True, shell=True)
        if result.returncode == 0:
            print("Deployment successful!")
        else:
            print("Deployment failed. Details:")
            print(result.stdout)
            print(result.stderr)
    except Exception as e:
        print("An unexpected error occurred during deployment:")
        print(e)
    
def job_type_merge_quests_weekly(is_hourly):
    print("Executing curl fetch cmd")
    query_quest_stats()
    print("Executing merge into DB")
    merge_in_db_quest_status(is_temp_data=is_hourly)
    print("Executing html generate")
    if gen_new_html() == True:
        print("Executing deployment")
        deploy_firebase()
    print(datetime.now(), 'Executed : job_type_merge_quests_weekly ', is_hourly)

def job_type_merge_war_daily(is_hourly):
    print("Executing curl fetch cmd")
    query_guild_members()
    print("Executing merge into DB")
    merge_in_db_guild_members(is_temp_data=is_hourly)
    print("Executing html generate")
    if gen_new_html() == True:
        print("Executing deployment")
        deploy_firebase()
    print(datetime.now(), 'Executed : job_type_merge_war_daily', is_hourly)

def job_type_reset_war_logs_daily():
    print("Resetting war logs")
    logs_today = get_war_map_logs()
    with open("logs_yesterday.txt", "w", encoding="utf-8") as file:
        file.write(logs_today)
    reset_war_logs()

def job_type_update_war_logs():
    gen_war_map_logs()
#    print("Executing html generate")
#    if gen_new_html() == True:
#        print("Executing deployment")
#        deploy_firebase()
    
def cleanup_old_runs(already_ran_today, already_ran_thishour, already_ran_thisminute, now):
    today_str = str(now.date())
    hour_str = now.strftime("%Y-%m-%d_%H")
    minute_str = now.strftime("%Y-%m-%d_%H_%M")

    already_ran_today.difference_update({key for key in already_ran_today if not key.endswith(today_str)})
    already_ran_thishour.difference_update({key for key in already_ran_thishour if not key.endswith(hour_str)})
    already_ran_thisminute.difference_update({key for key in already_ran_thisminute if not key.endswith(minute_str)})
    
async def run():
    already_ran_today = set()
    already_ran_thishour = set()
    already_ran_thisminute = set()

    while True:
        now = datetime.now()
        
        # DAILY JOBS
        for job in DAILY_JOBS:
            job_key = f'{job["job_type"]}_{now.date()}'
            if should_run_daily_now(job, now) and job_key not in already_ran_today:
                print(f"Running job: {job['job_type']} at {now}")
                
                if job['job_type'] == "merge_war":
                    job_type_merge_war_daily(False)

                if job['job_type'] == "merge_quests":
                    job_type_merge_quests_weekly(False)

                if job['job_type'] == "reset_logs":
                    job_type_reset_war_logs_daily()

                already_ran_today.add(job_key)

        # HOURLY JOBS
        for job in HOURLY_JOBS:
            job_key = f'{job["job_type"]}_{now.strftime("%Y-%m-%d_%H")}'
            if should_run_hourly_now(job, now) and job_key not in already_ran_thishour:
                print(f"Running hourly job: {job['job_type']} at {now}")
                if job['job_type'] == "temporary_war":
                    job_type_merge_war_daily(True)
                elif job['job_type'] == "temporary_quests":
                    job_type_merge_quests_weekly(True)
                already_ran_thishour.add(job_key)

        # MINUTE JOBS
        for job in MINUTE_JOBS:
            job_key = f'{job["job_type"]}_{now.strftime("%Y-%m-%d_%H_%M")}'
            if should_run_minute_now(job, now) and job_key not in already_ran_thisminute:
                print(f"Running minute job: {job['job_type']} at {now}")
                if job['job_type'] == "war_logs":
                    job_type_update_war_logs()
                already_ran_thisminute.add(job_key)

        cleanup_old_runs(already_ran_today, already_ran_thishour, already_ran_thisminute, now)
        
        time.sleep(30)

# Entry
if __name__ == "__main__":
    asyncio.run(run())
