import json
import os
import shutil

FILENAME = 'users.json'

def move_file_with_collision(src, dest_dir, only_copy = False):
    """
    Moves a file from src to dest_dir. If a file with the same name exists in dest_dir,
    appends a counter to the filename to avoid collision.
    
    Args:
        src (str): The source file path.
        dest_dir (str): The destination directory path.
    
    Returns:
        str: The final destination file path.
    """
    if not os.path.isfile(src):
        print(f"Source file {src} does not exist.")
        return None

    if not os.path.isdir(dest_dir):
        os.makedirs(dest_dir, exist_ok=True)

    base_name = os.path.basename(src)
    dest_path = os.path.join(dest_dir, base_name)
    
    # If file exists at destination, rename the file by appending a counter
    if os.path.exists(dest_path):
        name, ext = os.path.splitext(base_name)
        counter = 1
        while os.path.exists(dest_path):
            dest_path = os.path.join(dest_dir, f"{name}_{counter}{ext}")
            counter += 1

    if only_copy == True:
        shutil.copy(src, dest_path)
    else:
        shutil.move(src, dest_path)
        
    return dest_path
    
def load_users(filename=FILENAME):
    """Load users from a JSON file. Returns a dictionary with user IDs as keys."""
    if not os.path.exists(filename):
        return {}
    with open(filename, 'r') as file:
        try:
            data = json.load(file)
        except json.JSONDecodeError:
            data = {}
    return data

def save_users(users, filename=FILENAME):
    """Save the users dictionary to a JSON file."""
    with open(filename, 'w') as file:
        json.dump(users, file, indent=4)

def get_user(users, user_id, copy_from_user):
    """Return the user with the given id, or None if not found."""
    ret = users.get(user_id, None)
    
    # create new user if none was found
    if ret == None:
        ret = {
            "id": user_id,
            "created_on": "",
            "joined_on": "",
            "last_visit": "",
            "role": "",
            "Name": "",
            "Country": "",
            "TimeZone": "",
            "ChoosenLanguage": "",
            "summary_power": 0,
            "guild_might": 0,
            "spent_elixir": 0,
            "war_played": 0,
            "war_skipped": 0,
            "quests_completed": 0,
            "quests_dropped": 0,
            "score": 0,
            "quests_weekly_good": 0,
            "quests_weekly_fails": 0,
            "fame":0,
        }
    
    # check missing or incomplete fields
    if ret.get("created_on","") == "" and copy_from_user.get("created_on", "") != "": 
        ret["created_on"] = copy_from_user.get("created_on", "")
    if ret.get("joined_on","") == "" and copy_from_user.get("joined_on", "") != "": 
        ret["joined_on"] = copy_from_user.get("joined_on", "")
    if copy_from_user.get("last_visit", "") != "": 
        ret["last_visit"] = copy_from_user.get("last_visit", "")
    if copy_from_user.get("role", "") != "": 
        ret["role"] = copy_from_user.get("role", "")
    if copy_from_user.get("Name", "") != "": 
        ret["Name"] = copy_from_user.get("Name", "")
    if ret.get("Country","") == "" and copy_from_user.get("Country", "") != "": 
        ret["Country"] = copy_from_user.get("Country", "")
    if ret.get("TimeZone","") == "" and copy_from_user.get("TimeZone", "") != "": 
        ret["TimeZone"] = copy_from_user.get("TimeZone", "")
    if ret.get("ChoosenLanguage","") == "" and copy_from_user.get("ChoosenLanguage", "") != "": 
        ret["ChoosenLanguage"] = copy_from_user.get("ChoosenLanguage", "")
    if copy_from_user.get("guild_might", 0) != "": 
        ret["guild_might"] = copy_from_user.get("guild_might", 0)
    if copy_from_user.get("fame", 0) != "": 
        ret["fame"] = copy_from_user.get("fame", 0)
    
    return ret

def update_user(users, new_user):
    """
    Update (or add) a user.
    If the user already exists, merge the new data into the existing user.
    """
    user_id = new_user.get('id')
    if not user_id:
        raise ValueError("User object must have an 'id' field.")

    if user_id in users:
        # Merge new data with existing data
        users[user_id].update(new_user)
    else:
        # Add new user
        users[user_id] = new_user
    return users

def merge_in_db_quest_status(users = {}):
    if users == {}:
        users = load_users()
    input_file_name = "quest_stats.json"
    quest_stats = load_users(input_file_name)
    # no file to process
    if quest_stats == {}:
        return users
    quest_stats = quest_stats.get("result",{}).get("quest_stats",[])
    for quest_user in quest_stats:
        '''{
                "profile_id":"WEBGLGUEST7aa0f3bcbfe27de482d06534c414e379",
                "quests_completed":44,
                "quests_dropped":0,
                "score":3860,
                "warlord_id":7007,
                "NameBit":{
                   "Name":"wil",
                   "Prefix":"",
                   "Country":"TW",
                   "medalId":-1,
                   "TimeZone":"08:00:00",
                   "isSavable":true,
                   "medalValue":-1,
                   "InitialCountry":"TW",
                   "ChoosenLanguage":"zh-tw",
                   "CountryIsStatic":false
                },
                "warlord_promote":98
             },
        '''
        namebit = quest_user.pop("NameBit")
        quest_user.update(namebit)
        
        # get existing stats
        db_user = get_user(users, quest_user.get("profile_id", ""), quest_user)
        
        # update with new values
        db_user["quests_completed"] += quest_user.get("quests_completed", 0)
        db_user["quests_dropped"] += quest_user.get("quests_dropped", 0)
        db_user["score"] += quest_user.get("score", 0)
        if quest_user.get("quests_completed", 0) < 10 or quest_user.get("score", 0) < 1000:
            db_user["quests_weekly_fails"] += 1
        else:
            db_user["quests_weekly_good"] += 1
          
        # save to inmem db
        users = update_user(users, db_user)

    # always backup ?
    move_file_with_collision(FILENAME, "archive", True)
    
    # just in case save to file
    save_users(users)
    
    # remove this processed file 
    move_file_with_collision(input_file_name, "archive")
    
    # return the updated users
    return users

def merge_in_db_guild_members(users = {}):
    if users == {}:
        users = load_users()
    input_file_name = "guild_members.json"
    guild_members = load_users(input_file_name)
    # no file to process
    if guild_members == {}:
        return users
    guild_members = guild_members.get("result",{}).get("members",[])
    for guild_member in guild_members:
        '''{
            "guild_id":3118,
            "profile_id":"Steam76561197991263792",
            "joined_on":"2025-01-27T00:36:10.510767",
            "role":"member",
            "was_guild_master":null,
            "locked_gw_till":null,
            "locked_gs_till":null,
            "locked_regatta_till":null,
            "NameBit":{
               "Name":"Maughold",
               "Prefix":"",
               "Country":"CA",
               "medalId":-1,
               "TimeZone":"-04:00:00",
               "isSavable":true,
               "medalValue":-1,
               "InitialCountry":"CA",
               "ChoosenLanguage":"English",
               "CountryIsStatic":false
            },
            "id":"Steam76561197991263792",
            "created_on":"2025-01-27T00:36:10.510767",
            "type":"member",
            "remaining_power":2823603,
            "summary_power":2970740,
            "guild_might":2823603,
            "fame":5495,
            "last_visit":"2025-04-02T15:34:05.347571",
            "warlord_id":3002,
            "warlord_promote":70,
            "spent_elixir":0
         },
        '''
        namebit = guild_member.pop("NameBit")
        guild_member.update(namebit)
        
        # get existing stats
        db_user = get_user(users, guild_member.get("profile_id", ""), guild_member)
        
        # update with new values
        db_user["fame"] += guild_member.get("fame", 0)
        db_user["spent_elixir"] += guild_member.get("spent_elixir", 0)
        db_user["summary_power"] += guild_member.get("summary_power", 0)
        if guild_member.get("summary_power", 0) == 0:
            db_user["war_skipped"] += 1
        else:
            db_user["war_played"] += 1
          
        # save to inmem db
        users = update_user(users, db_user)

    # always backup ?
    move_file_with_collision(FILENAME, "archive", True)

    # just in case save to file
    save_users(users)
    
    # remove this processed file 
    move_file_with_collision(input_file_name, "archive")
    
    # return the updated users
    return users
    
if __name__ == "__main__":
    users = merge_in_db_quest_status()
    users = merge_in_db_guild_members(users)    