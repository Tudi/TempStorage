from update_users_db import load_users
from datetime import datetime

def construct_table_header():
    row = "<thead><tr style=\"background-color: purple; color: white;\">"
    row += "<th data-type=\"string\">Player Name</th>"
    row += "<th data-type=\"number\">War days skipped</th>"
    row += "<th data-type=\"number\">Weekly quest requirement fails</th>"
    row += "<th data-type=\"number\">wars palced</th>"
    row += "<th data-type=\"number\">wars skipped</th>"
    row += "<th data-type=\"number\">fuel spent</th>"
    row += "<th data-type=\"number\">total quests completed</th>"
    row += "<th data-type=\"number\">total quests failed</th>"
    row += "<th data-type=\"number\">total quest points</th>"
    row += "<th data-type=\"number\">weekly quest requirements passed</th>"
    row += "<th data-type=\"number\">weekly quest requirements failed</th>"
    row += "<th data-type=\"number\">Fame</th>"
    row += "</tr></thead>"
    return row

def construct_table_row(user):
    row = "<tr>"
    row += "<td>" + str(user["Name"]) + "</td>"
    row += "<td>" + str(user["war_skipped"]) + "</td>"
    row += "<td>" + str(user["quests_weekly_fails"]) + "</td>"

    row += "<td>" + str(user["war_played"]) + "</td>"
    row += "<td>" + str(user["war_skipped"]) + "</td>"
    row += "<td>" + str(user["spent_elixir"]) + "</td>"
    row += "<td>" + str(user["quests_completed"]) + "</td>"
    row += "<td>" + str(user["quests_dropped"]) + "</td>"
    row += "<td>" + str(user["score"]) + "</td>"
    row += "<td>" + str(user["quests_weekly_good"]) + "</td>"
    row += "<td>" + str(user["quests_weekly_fails"]) + "</td>"
    row += "<td>" + str(user["fame"]) + "</td>"
    row += "</tr>"
    return row
    
table_sorting_script = '''<script>
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
      
      // Determine sort order: toggle if same column, default to ascending otherwise.
      const currentIsAscending = table.dataset.sortColumn == columnIndex && table.dataset.sortDirection == "asc";
      const newDirection = currentIsAscending ? "desc" : "asc";
      table.dataset.sortColumn = columnIndex;
      table.dataset.sortDirection = newDirection;
      
      rows.sort((a, b) => {
        const cellA = a.querySelectorAll("td")[columnIndex].textContent.trim();
        const cellB = b.querySelectorAll("td")[columnIndex].textContent.trim();
        
        let comparison = 0;
        if (type === "number") {
          comparison = parseFloat(cellA) - parseFloat(cellB);
        } else {
          // Case-insensitive string comparison
          comparison = cellA.localeCompare(cellB, undefined, { sensitivity: 'base' });
        }
        
        return newDirection === "asc" ? comparison : -comparison;
      });
      
      // Clear existing rows and re-append sorted rows
      rows.forEach(row => tbody.removeChild(row));
      rows.forEach(row => tbody.appendChild(row));
    }
  </script>'''
  
def gen_new_html():
    users = load_users()
    print("Loaded " + str(len(users)) + " users")
    # filter out users that did not log in past 2 days. We will kick these
    now = datetime.now()
    active_users = {}
    for user in users:
        user = users[user]
#        print(user)
        last_visit = datetime.fromisoformat(user["last_visit"])
        days_since_online = (now - last_visit).days
        if days_since_online >= 2:
            continue
        active_users[user["id"]] = user
        
    print("have " + str(len(active_users)) + " active users")
    html_content = "<a href=\"https://discord.com/channels/1306315094724186133/1306744587418603622\">Join us on Discord ( open a ticket )</a>"
    html_content += "<table style=\"text-align:center;\" id=\"userTable\">"
    html_content += construct_table_header()
    
    for user in active_users:
        user = active_users[user]
        #print(user["id"])
        html_content += construct_table_row(user)
        
    html_content += "</table>"
    
    html_content += table_sorting_script
    
    with open("./public/index.html", 'w') as file:
        file.write(html_content)
     
if __name__ == "__main__":    
    gen_new_html()