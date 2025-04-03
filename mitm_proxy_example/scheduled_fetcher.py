import asyncio
import json
import os
from datetime import datetime
import time
from construct_goddess_req import query_quest_stats,query_guild_members
from update_users_db import merge_in_db_quest_status, merge_in_db_guild_members
from gen_users_html import gen_new_html
import subprocess

JOBS = [
    {
        "run_time": "20:50",
        "days": ["mon", "tue", "wed", "thu", "fri", "sat", "sun"],
        "job_type": "fetch_war"
    },
    {
        "run_time": "12:55",
        "days": ["fri"],
        "job_type": "fetch_quests"
    }
]

test_runs_to_make = 2
test_runs_made = 0

def should_run_now(job, now):
    global test_runs_made, test_runs_to_make
    if test_runs_made < test_runs_to_make:
        test_runs_made += 1
        #return True
    now_str = now.strftime("%H:%M")
    today = now.strftime("%a").lower()
    return job["run_time"] == now_str and today in job["days"]

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
    
def job_type_fetch_quests_weekly():
    query_quest_stats()
    merge_in_db_quest_status()
    gen_new_html()
    deploy_firebase()

def job_type_fetch_war_daily():
    query_guild_members()
    merge_in_db_guild_members()
    gen_new_html()
    deploy_firebase()
    
async def run():
    already_ran_today = set()

    while True:
        now = datetime.now()
        for job in JOBS:
            job_key = f'{job["job_type"]}_{now.date()}'
            if should_run_now(job, now) and job_key not in already_ran_today:
                print(f"Running job: {job['job_type']} at {now}")
                
                if job['job_type'] == "fetch_war":
                    job_type_fetch_war_daily()

                if job['job_type'] == "fetch_quests":
                    job_type_fetch_quests_weekly()

                already_ran_today.add(job_key)

        time.sleep(30)

# Entry
asyncio.run(run())
