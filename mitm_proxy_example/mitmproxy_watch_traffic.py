from mitmproxy import http
from urllib.parse import urlparse, parse_qs
import json
import gzip
from io import BytesIO
import datetime
import re

# Target domain and paths
TARGET_HOST = "ga.chickgoddess.com"
TARGET_PATHS = [
    "/gs_api/pvp/start",
    "/gs_api/pvp/status",
]

IGNORE_PATHS = [
]

HEADER_NEEDS_VAUES = [
    "pnk-device-id",
    "pnk-login-id",
    "pnk-config-ctx",
    "pnk-session-id"
]

PATHS_DUMP_HEADER_BODY = [
    "/gs_api/regatta/get_quests_stats",
    "/gs_api/guilds/get_members",
]

# Dictionary to track how many times each unique URL has been logged
logged_urls = {}

def is_target_request(req) -> bool:
    if req.pretty_host != TARGET_HOST:
        return False
    # ignore these
    if any(req.path.startswith(p) for p in IGNORE_PATHS):
        return False
    # process everything to catch new packets
    return True

# we need all specific header variables in order to dump them, ignore incomplete packets
def can_dump_headers(req):
    values_found = 0
    for k, v in req.headers.items():
        if k in HEADER_NEEDS_VAUES:
            values_found += 1
    if values_found == len(HEADER_NEEDS_VAUES):
        return True
    return False

# filter out already known packet types and only show those we are interested in
def should_dump_req_info(req):
    if any(req.path.startswith(p) for p in PATHS_DUMP_HEADER_BODY):
        return True
    return False

def dump_req_headers_visually(req):
    print("Headers:")
    for k, v in req.headers.items():
        print(f"  {k}: {v}")

def dump_req_resp_body_visually(req_resp):
    content_encoding = req_resp.headers.get("content-encoding", "").lower()
    content_type = req_resp.headers.get("content-type", "").lower()

    if req_resp.raw_content:
        try:
            if "gzip" in content_encoding:
                # Attempt to decompress gzip
                buf = BytesIO(req_resp.raw_content)
                decompressed = gzip.GzipFile(fileobj=buf).read().decode("utf-8")
            else:
                decompressed = req_resp.get_text()

            if "application/json" in content_type:
                json_data = json.loads(decompressed)
                print("JSON Request Body:")
                print(json.dumps(json_data, indent=2))
            else:
                print("Text Request Body:")
                print(decompressed[:500])
        except Exception as e:
            print(f"[!] Could not decode request body: {e}")
            print("Raw (first 100 bytes, hex):")
            print(req_resp.raw_content[:100].hex(" "))

# we need these headers for request injection
def dump_headers_to_file(req):
    # gather all req headers
    headers_as_string = ""
    for k, v in req.headers.items():
        headers_as_string += f"{k}:{v}\n"
    # write req headers to file
    with open("req_header.txt", "wt") as f:
        f.write(headers_as_string)

# dump with overwrite these responses        
def dump_response_body(flow):
    # gen file name based on url
    url = flow.request.pretty_url
    path_after_gs_api = url.split('gs_api/', 1)[-1]
    path_after_gs_api = path_after_gs_api.replace('/', '_')
    safe_filename = re.sub(r'[<>:"/\\|?*]', '_', path_after_gs_api)
    # dump current timestamp and request body to file
    with open(safe_filename + ".txt", "wt", encoding="utf-8") as f:
        f.write(str(datetime.datetime.now()))
        f.write("\n")
        f.write(flow.response.get_text())

def log_request_response(flow):
    """
    Append the URL, request info and response body to a log file.
    Each unique URL is logged only 10 times.
    """
    global logged_urls
    url = flow.request.pretty_url

    # Initialize counter for new URLs
    if url not in logged_urls:
        logged_urls[url] = 0

    # Only log if we haven't hit the 10 times threshold
    if logged_urls[url] < 10:
        try:
            with open("traffic_log.txt", "a", encoding="utf-8") as f:
                f.write("Timestamp: " + str(datetime.datetime.now()) + "\n")
                f.write("URL: " + url + "\n")
                f.write("Request Method: " + flow.request.method + "\n")
                f.write("Request Headers:\n")
                for k, v in flow.request.headers.items():
                    f.write(f"  {k}: {v}\n")
                f.write("Request Body:\n")
                f.write(flow.request.get_text() + "\n")
                f.write("Response Body:\n")
                f.write(flow.response.get_text() + "\n")
                f.write("=" * 80 + "\n")
            logged_urls[url] += 1
        except Exception as e:
            print(f"[!] Failed to log URL {url}: {e}")

# log requests so we can get the latest session parameters that we will use to issue new requests
def request(flow: http.HTTPFlow):
    req = flow.request
    if is_target_request(req):
        print(f"Req: {req.method} URL: {req.pretty_url}")
        # we only care about a couple of packets to see visually
        if should_dump_req_info(req):
            dump_req_headers_visually(req)
            dump_req_resp_body_visually(req)
        # if we have all the required info in the header, update our session id and other variables
        if can_dump_headers(req):
            dump_headers_to_file(req)

# check response content to be able to parse them         
def response(flow: http.HTTPFlow):
    req = flow.request
    if is_target_request(req):
        print(f"Resp: {flow.response.status_code} Request URL: {req.pretty_url}")
        # we only care about a couple of packets to see visually
        if should_dump_req_info(req):
            dump_req_resp_body_visually(flow.response)
            dump_response_body(flow)
        # Log each unique URL (request + response) 10 times
        log_request_response(flow)
