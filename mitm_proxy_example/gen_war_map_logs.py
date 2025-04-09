from datetime import datetime
import time
import os
import json
from construct_goddess_req import query_quest_stats, query_guild_members, query_war_map
from utils import format_large_number

# Global state
prev_map = {}
prev_member = {}
logs_today = ""
loaded_content_from_file = 0

STATE_FILE = "war_log_state.json"

def save_state():
    global prev_map, prev_member, logs_today
    with open(STATE_FILE, 'w') as f:
        json.dump({
            "prev_map": prev_map,
            "prev_member": prev_member,
            "logs_today": logs_today
        }, f)

def load_state():
    global prev_map, prev_member, logs_today
    if not os.path.exists(STATE_FILE):
        return
    with open(STATE_FILE, 'r') as f:
        try:
            state = json.load(f)
            prev_map = state.get("prev_map", {})
            prev_member = state.get("prev_member", {})
            logs_today = state.get("logs_today", "")
        except json.JSONDecodeError:
            pass  # If corrupt, skip loading

def reset_war_logs():
    global prev_map, prev_member, logs_today
    prev_map = {}
    prev_member = {}
    logs_today = ""
    save_state()

def load_war_logs(filename="war_map.json"):
    """Load users or map data from a JSON file. Returns a dictionary."""
    if not os.path.exists(filename):
        return {}
    with open(filename, 'r') as file:
        try:
            data = json.load(file)
        except json.JSONDecodeError:
            data = {}
    return data

def cell_x_y_to_coord(cell_x,cell_y):
    cell_x = int(cell_x) + 1
    y_to_letter = ['A','B','C','D','E','F','G','H','I','J','K','L','M']
    cell_y = y_to_letter[int(cell_y)]
    return str(cell_y)+str(cell_x)
    
def gen_war_map_logs():
    global logs_today, prev_map, prev_member, loaded_content_from_file

    try:
        # in case we had to restart our program
        if loaded_content_from_file == 0:
            loaded_content_from_file = 1
            load_state()
            
        # Fetch map status
        query_war_map()
        map_resp = load_war_logs("war_map.json")

        # Detect power changes on the map
        logs_now = ""
        for cell in map_resp.get("result", {}).get("cells", []):
            for force in cell.get("forces", []):
                if "arasaka tech" in str(force.get("guild_name", "")).lower():
                    cell_x = str(force.get("cell_x"))
                    cell_y = str(force.get("cell_y"))
                    key = f"{cell_x}_{cell_y}"
                    current_power = force.get("power", 0)
                    previous_power = prev_map.get(key, 0)
                    if current_power > previous_power:
                        logs_now += f"{cell_x_y_to_coord(cell_x,cell_y)} : {format_large_number(current_power - previous_power)}<br>"
                        prev_map[key] = current_power

        if len(logs_now) == 0:
            return  # no change

        # Fetch user status
        query_guild_members()
        guild_members = load_war_logs("guild_members.json")

        # Detect power gains
        player_logs = ""
        for member in guild_members.get("result", []).get("members", []):
            profile_id = member.get("profile_id")
            name = member.get("NameBit", {}).get("Name", "Unknown")
            power = member.get("summary_power", 0)
            previous_power = prev_member.get(profile_id, 0)
            if power > previous_power:
                prev_member[profile_id] = power
                player_logs += f"{name}&nbsp;"

        if len(player_logs) == 0:
            return  # no boosting activity

        # Append new table row to logs
        now = datetime.now()
        logs_now = f"<tr><td>{now.strftime('%H:%M')}</td><td>{logs_now}</td><td>{player_logs}</td></tr>"
        logs_today += logs_now
        
        print(f"war map logs size {len(logs_now)}")
        
        save_state()

    except Exception as e:
        print(f"[ERROR] gen_war_map_logs failed: {e}")

def get_war_map_logs():
    return logs_today
