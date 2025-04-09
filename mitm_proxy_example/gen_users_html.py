import os
from update_users_db import load_users
from datetime import datetime
from gen_war_map_logs import get_war_map_logs
from utils import format_large_number, write_file_if_changed

def get_days_since_last_seen(user):
    if (user.get("last_visit", 0) or 0) != 0:
        last_visit = datetime.fromisoformat(user["last_visit"])
        now = datetime.now()
        hour_granularity = 4
        days_since_online = int((now - last_visit).total_seconds() / (60 * hour_granularity)) # to not trigger a page update more than every X hour
        days_since_online = days_since_online / (60 * 24 / hour_granularity)
        return f"{days_since_online:.2f}"
    return ""
        
def construct_table_header():
    row = "<thead><tr style=\"background-color: purple; color: white;\">"
    row += "<th style=\"border:none;\" data-type=\"string\">Player Name</th>"
    
    row += "<th style=\"border:none;\" data-type=\"number\">War placed today</th>"
    row += "<th style=\"border:none;\" data-type=\"number\">Fuel today</th>"
    row += "<th style=\"border:none;\" data-type=\"number\">Heli quests</th>"
    row += "<th style=\"border:none;\" data-type=\"number\">Heli points</th>"
    row += "<th style=\"border:none;\" data-type=\"number\">Last seen</th>"
    
    row += "<th style=\"border:none;\" data-type=\"number\">Total Wars placed</th>"
    row += "<th style=\"border:none;\" data-type=\"number\">Total Wars skipped</th>"
    row += "<th style=\"border:none;\" data-type=\"number\">Total Fuel spent</th>"
    row += "<th style=\"border:none;\" data-type=\"number\">Total quests</th>"
    #row += "<th data-type=\"number\">total quests failed</th>"
    row += "<th style=\"border:none;\" data-type=\"number\">Total quest points</th>"
    row += "<th style=\"border:none;\" data-type=\"number\">Weekly quest requirements passed</th>"
    row += "<th style=\"border:none;\" data-type=\"number\">Weekly quest requirements failed</th>"
    row += "<th style=\"border:none;\" data-type=\"number\">PVP might</th>"
    row += "<th style=\"border:none;\" data-type=\"number\">War might</th>"
    row += "<th style=\"border:none;\" data-type=\"number\">War might min</th>"
    row += "</tr></thead>"
    return row

def construct_table_row(user):
    row = "<tr>"
    row += "<td>" + str(user["Name"]) + "</td>"
    # stats for today so far
    row += "<td>" + format_large_number(user.get("war_played_temp",0)) + "</td>"
    row += "<td>" + format_large_number(user.get("spent_elixir_temp",0)) + "</td>"
    row += "<td>" + format_large_number(user.get("quests_completed_temp",0)) + "</td>"
    row += "<td>" + format_large_number(user.get("score_temp",0)) + "</td>"
    row += "<td>" + get_days_since_last_seen(user) + "</td>"
    # overall stats
    row += "<td>" + format_large_number(user["war_played"] + user.get("war_played_temp",0)) + "</td>"
    row += "<td>" + format_large_number(user["war_skipped"] + user.get("war_skipped_temp",0)) + "</td>"
    row += "<td>" + format_large_number(user["spent_elixir"] + user.get("spent_elixir_temp",0)) + "</td>"
    row += "<td>" + format_large_number(user["quests_completed"] + user.get("quests_completed_temp",0)) + "</td>"
    #row += "<td>" + str(user["quests_dropped"] + user.get("quests_dropped_temp",0)) + "</td>"
    row += "<td>" + format_large_number(user["score"] + user.get("score_temp",0)) + "</td>"
    row += "<td>" + format_large_number(user["quests_weekly_good"]) + "</td>"
    row += "<td>" + format_large_number(user["quests_weekly_fails"]) + "</td>"
    row += "<td>" + format_large_number(user["fame"]) + "</td>" # this is pvp might
    row += "<td>" + format_large_number(user["guild_might"]) + "</td>" # this is war might
    row += "<td>" + format_large_number((user.get("guild_might_first",0) or 0)) + "</td>" # this is war might
    row += "</tr>"
    return row
    
page_styles = r'''<style>
  .zebra-table {
    border-collapse: collapse;
    width: 100%;
  }

  .zebra-table td, .zebra-table th {
    border: 1px solid #ddd; /* Optional, for cell borders */
    padding: 8px;
  }

  .zebra-table tr:nth-child(odd) {
    background-color: rgb(240, 240, 240); /* Light gray */
  }

  .zebra-table tr:nth-child(even) {
    background-color: rgb(255, 255, 255); /* White */
  }		
</style>
'''

table_sorting_script = r'''<script>
    document.addEventListener('DOMContentLoaded', () => {
      const table = document.getElementById("userTable");
      const headers = table.querySelectorAll("th");
      
      headers.forEach((header, index) => {
        header.addEventListener("click", () => {
          sortTableByColumn(table, index, header.dataset.type);
        });
      });
    });
    
    function sortTableByColumn(table, columnIndex, type) {
      const tbody = table.querySelector("tbody");
      const rows = Array.from(tbody.querySelectorAll("tr"));

      const currentIsAscending = table.dataset.sortColumn == columnIndex && table.dataset.sortDirection == "asc";
      const newDirection = currentIsAscending ? "desc" : "asc";
      table.dataset.sortColumn = columnIndex;
      table.dataset.sortDirection = newDirection;

      rows.sort((a, b) => {
        const cellA = a.querySelectorAll("td")[columnIndex].textContent.trim();
        const cellB = b.querySelectorAll("td")[columnIndex].textContent.trim();

        let valA, valB;

        if (type === "number") {
          valA = parseSmartNumber(cellA);
          valB = parseSmartNumber(cellB);
        } else {
          valA = cellA.toLowerCase();
          valB = cellB.toLowerCase();
        }

        let comparison = 0;
        if (type === "number") {
          comparison = valA - valB;
        } else {
          comparison = valA.localeCompare(valB);
        }

        return newDirection === "asc" ? comparison : -comparison;
      });

      rows.forEach(row => tbody.removeChild(row));
      rows.forEach(row => tbody.appendChild(row));
    }

    function parseSmartNumber(value) {
      const multiplier = {
        K: 1_000,
        M: 1_000_000,
        B: 1_000_000_000
      };

      const cleaned = value.replace(/,/g, '').trim(); 
      const match = cleaned.match(/^([\d.]+)\s*([KMB])?$/i); 

      if (!match) return parseFloat(cleaned);

      const num = parseFloat(match[1]);
      const suffix = match[2] ? match[2].toUpperCase() : '';

      return num * (multiplier[suffix] || 1);
    }

  </script>'''
  
def gen_discord_link():
#    return "<a href=\"https://discord.com/channels/1306315094724186133/1306744587418603622\">Join us on Discord ( open a ticket )</a><br>"
    return "<a href=\"https://discord.gg/yUXTj4X8\">Join us on Discord ( open a ticket )</a><br>"
    
def gen_guild_rules():
    rules_html = '''
<B><u>Guild Rules that might get you kicked ( last updated 2025/04/08 ): </u></B><br>
    - Complete at least 10 Hely quests per week<br>
    - Earn at least 1000 Heli quest points per week<br>
    - Only place on tiles marked with attack icon. Do not attack randomly ! If you need to place ( going to sleep), defend existing tiles. Defense is just as important as attack<br>
    - Stay at low war power. Avoid upgrades that increases your war might ! <br>
    - Buy fuel, and use fuel in war<br>
    - Best time to place for war is turn 3<br>
    - War turn 1 and 2, we will take MAX 3 tiles. Only in turn 3 we will take more tiles if possible<br>
    - Joining Discord is advised! Life chatting / help<br>
    - We accept smurf accounts with leftover Emeryx to help boost Heli quests<br>
<br>    
<B><u>War for the blind : </u></B><br>
    - this is the "war turn indicator". "clash 2" means we are in turn 2. Max 3 tiles are allowed ! 3 hours remain until tile ownership change. <br>
<img src=\"https://i.postimg.cc/SxCnJyjh/war-turn-marker.png\"><br>
    - this is a tile you are allowed to attack (the attack icon). Nothing else. <br>
<img src=\"https://i.postimg.cc/9fBzYJGg/tile-marked-for-attack.png\"><br>
    - if you can't wait until turn 3. You can defend anytime these tiles. <br>
<img src=\"https://i.postimg.cc/vBqBpT2W/tile-you-can-defend.png\"><br>
    - this is an "uprising" tile that we do not place troops in. After uprising it will become neutral<br>
<img src=\"\"><br>
<br>    
<B><u>War for beginners: </u></B><br>
    - guild gets daily rewards by having tiles captured<br>
    - you get rewards by placing units as defenders or as attackers (same reward). Defending tiles ensure we get rewards.<br>
    - again : you get rewards by placing units, does not matter if they capture a tile, defend a tile, tile gets captured or not.<br>
    - reward amount depends on number of tiles, ranking of the aliance, your might rank inside the aliance ( this is recalculated daily based on fuel usage )<br>
    - every day war has 3 turns. Every 8 hours a new turn begins<br>
    - at the end of every turn. tile combat comences and tile ownership gets recalculated<br>
    - in turn 1 and 2, our goal is to take max 3 tiles !!<br>
    - if you take more than 3 tiles, tiles will randomly trigger uprising and you will loose the tile. We might not be able to recapture these tiles !<br>
    - tiles that triggered uprising will use the defended troop might. Sometimes they become impossible to recapture. If we attacked this tile with 20M might, we will need 21M might to recapture it !<br>
    - war echo event is not worth waiting on. But you do not want to upgrade troops anyway.<br>
<br>
<B><u>Player activity is tracked. If a new spot is needed, the worst player will be kicked (rules broken)</u></B><br>
    '''
    return rules_html
    
def gen_war_map_logs():
    war_map_logs_today = get_war_map_logs()
    if len(war_map_logs_today) > 0:
        table_content = r'''<table border="1">'''
        table_content += "<tr><td>Time</td><td>Cells</td><td>Players</td></tr>"
        table_content += war_map_logs_today
        table_content += "</table>"
        return table_content
    return ""
    
def gen_new_html():
    users = load_users()
    #print("Loaded " + str(len(users)) + " users")
    # filter out users that did not log in past 2 days. We will kick these
    now = datetime.now()
    active_users = {}
    for user in users:
        user = users[user]
#        print(user)
        if (user.get("last_visit", 0) or 0) != 0:
            last_visit = datetime.fromisoformat(user["last_visit"])
            days_since_online = (now - last_visit).days
            if days_since_online >= 2:
                continue
        else:
            continue;
        active_users[user["id"]] = user
        
    #print("have " + str(len(active_users)) + " active users")
    html_content = ""
    # link to our discord
    html_content += page_styles
    html_content += gen_discord_link()
    html_content += gen_guild_rules()
    html_content += gen_war_map_logs()
    html_content += "<table  class=\"zebra-table\" style=\"text-align:center;border-collapse:collapse;border-spacing:0;border:none;padding:0;\" id=\"userTable\">"
    html_content += construct_table_header()
    
    for user in active_users:
        user = active_users[user]
        #print(user["id"])
        html_content += construct_table_row(user)
        
    html_content += "</table>"
    
    html_content += table_sorting_script
    
    return write_file_if_changed("./public/index.html", html_content)
    
if __name__ == "__main__":    
    gen_new_html()
    
    