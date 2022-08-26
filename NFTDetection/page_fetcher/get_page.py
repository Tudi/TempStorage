from selenium import webdriver
from selenium.webdriver.firefox.options import Options
from selenium.webdriver.firefox.firefox_profile import FirefoxProfile
#from BeautifulSoup4 import BeautifulSoup
#import pandas as pd
import os
import math
import json
import requests
from argparse import ArgumentParser
import time
import urllib
import pyautogui
import sys

image_full_path="d:\\GitHub\\Rev3al"
keyboard_latency=0.4

def GetImageContent(url, saveto):
    try:
        try:
            filename, headers = urllib.request.urlretrieve(url, saveto)
            time.sleep(1)
        except:
            filename = ""
        if not os.path.exists(saveto):
            driver.get(url)
            pyautogui.hotkey('ctrl','s', interval=keyboard_latency)
            time.sleep(1)
            pyautogui.press('delete', interval=keyboard_latency)
            pyautogui.hotkey('ctrl','a', interval=keyboard_latency)
            pyautogui.press('delete', interval=keyboard_latency)
            pyautogui.write(saveto)
            time.sleep(keyboard_latency)
            pyautogui.click()
            #pyautogui.press('tab', interval=keyboard_latency)
            #pyautogui.press('tab', interval=keyboard_latency)
            #pyautogui.press('enter', interval=keyboard_latency)
            #pyautogui.press('esc', interval=keyboard_latency)
            #pyautogui.press('esc', interval=keyboard_latency)
    #        pyautogui.hotkey('alt', 's', interval=keyboard_latency)
    #        time.sleep(2)
        print("saved %s to %s" % (url,saveto))
    except:
        print("failed to fetch image %s" % url)
        sys.exit()
    exit

quality=1
collection="cryptopunks"
ROOT_WEBSITE="https://api.opensea.io/api/v1/assets?collection=" + collection + "&limit=50&format=json&cursor="
cursor=""
download_max_pages=20

ffoptions=Options()
ffoptions.set_preference("browser.download.folderList", 2)
ffoptions.set_preference("browser.download.manager.showWhenStarting", False) #deprecated ?
ffoptions.set_preference("browser.download.dir", ".\\images\\"+collection+"\\") #deprecated ?
ffoptions.set_preference("browser.download.lastDir", ".\\images\\"+collection+"\\")
ffoptions.set_preference("browser.helperApps.neverAsk.saveToDisk", "application/png") #not working ?

driver = webdriver.Firefox(options=ffoptions)
driver.maximize_window()
 
while 1>0 :
    driver.get(ROOT_WEBSITE+cursor)
    #element_content = driver.page_source
    #print(content)
    #time.sleep(25000)
    element=driver.find_element("id", "json")
    element_content = element.text
#    print(element_content)
    data_request = json.loads(element_content)
    for asset in data_request["assets"]:
        formatted_number = asset['token_id']
        if not os.path.exists(f"{image_full_path}\\images\\{collection}\\{formatted_number}.png"):
            if quality and asset["image_original_url"]:
                GetImageContent(asset["image_original_url"], f"{image_full_path}\\images\\{collection}\\{formatted_number}.png")
            else:
                if asset["image_url"]:
                    GetImageContent(asset["image_url"], f"{image_full_path}\\images\\{collection}\\{formatted_number}.png")
                else:
                    continue
    for asset in data_request["assets"]:
        formatted_number = f"{int(asset['token_id']):04d}"
        if not os.path.exists(f".\\images\\{collection}\\data\\{formatted_number}.json"):
            file = open(f".\\images\\{collection}\\data\\{formatted_number}.json", "w+")
            json.dump(asset, file, indent=3)
            file.close()
    cursor = data_request["next"]
    if len(cursor)<=0:
        printf("page was : " + element_content)
        print("Cursor was empty, exiting")
        break
    print("next cursor is %s" % cursor)
    download_max_pages = download_max_pages - 1
    if download_max_pages == 0:
        break
    time.sleep(0.25)

driver.close()