import uuid
import subprocess
from datetime import datetime, timezone
from typing import Dict
import os
import gzip
import json

def load_headers_from_file(filepath: str) -> Dict[str, str]:
    headers = {}
    with open(filepath, "r", encoding="utf-8") as file:
        for line in file:
            if ":" in line:
                key, value = line.strip().split(":", 1)
                headers[key.strip()] = value.strip()
    return headers


def generate_dynamic_headers(data) -> Dict[str, str]:
    now = datetime.now()
    utc_now = datetime.now(timezone.utc)
    return {
        "pnk-request-client-start-time": now.isoformat(timespec="microseconds"),
        "pnk-request-client-system-time": now.astimezone().isoformat(timespec="microseconds"),
        "pnk-request-client-system-time-utc": utc_now.isoformat(timespec="microseconds"),
        "pnk-request-id": str(uuid.uuid4()),
        "datahash": "",
        "content-length": len(data),
    }


def build_curl_command(method: str, url: str, headers: dict, data: str = None, output_file: str = "response.json") -> list:
    cmd = ["curl", "-vv", "-s", "-X", method.upper(), url, "-o", output_file]
    for key, value in headers.items():
        cmd.extend(["-H", f"{key}: {value}"])
    if method.upper() == "POST" and data:
        cmd.extend(["--data", data])
    return cmd


def execute_request(method: str, url: str, data: str = "{}", output_file: str = "response.json"):
    base_headers = load_headers_from_file("req_header.txt")
    
    # we do not need this one
    if base_headers.get("x-content-encoding", None) != None:
        del base_headers["x-content-encoding"]
    
    dynamic_headers = generate_dynamic_headers(data)
    
    headers = {**base_headers, **dynamic_headers}
    
    abs_path = os.path.abspath(output_file)
    cmd = build_curl_command(method, url, headers, data, abs_path)
    #print(f"executing : {cmd}")
    result = subprocess.run(cmd,capture_output=True,text=True)
    
    # because output is always in stderr ?
    if result.stderr:
        stdout_content = result.stderr
    if not result.stderr and result.stdout:
        stdout_content = result.stdout
    #print(stdout_content)
    
    if "Content-Encoding: gzip" in stdout_content:
        #print("File content is compressed, trying to uncompress it")
        with open(abs_path,"rb") as f:
            file_data = f.read()
        uncompressed_data = gzip.decompress(file_data)
        with open(abs_path,"wb") as f:
            f.write(uncompressed_data)
    
    if result.returncode != 0:
        print("Curl command failed with error:")
        print(result.stderr)
#    else:  
#        print(f"=> Response output : {abs_path}")

def query_guild_members(members=True, requests=False, recruits=False, invites=False, guild_id=None):
    url = "https://ga.chickgoddess.com/gs_api/guilds/get_members"
    
    body_dict = {
        "guild_id": guild_id,
        "members": members,
        "recruits": recruits,
        "requests": requests,
        "invites": invites
    }

    body = json.dumps(body_dict)
    
    execute_request("POST", url, data=body, output_file="guild_members.json")


def query_quest_stats():
    url = "https://ga.chickgoddess.com/gs_api/regatta/get_quests_stats"
    execute_request("POST", url, data="{}", output_file="quest_stats.json")


def query_war_map():
    url = "https://ga.chickgoddess.com/gs_api/guild_wars/get_map"
    execute_request("POST", url, data="{\"tick_number\":-1}", output_file="war_map.json")

if __name__ == "__main__":
    query_guild_members()
    query_quest_stats()
