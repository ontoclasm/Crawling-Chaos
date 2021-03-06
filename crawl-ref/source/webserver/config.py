import logging
from collections import OrderedDict

dgl_mode = True

bind_nonsecure = True # Set to false to only use SSL
bind_address = ""
bind_port = 8080

logging_config = {
#    "filename": "webtiles.log",
    "level": logging.INFO,
    "format": "%(asctime)s %(levelname)s: %(message)s"
}

password_db = "./webserver/passwd.db3"

static_path = "./webserver/static"
template_path = "./webserver/templates/"

# Path for server-side unix sockets (to be used to communicate with crawl)
server_socket_path = None # Uses global temp dir

# Server name, so far only used in the ttyrec metadata
server_id = ""

# Disable caching of game data files
game_data_no_cache = True

# Game configs
# %n in paths is replaced by the current username
games = OrderedDict([
    ("dcss-web-trunk", dict(
        name = "DCSS trunk",
        crawl_binary = "./crawl",
        rcfile_path = "./rcs/",
        macro_path = "./rcs/",
        morgue_path = "./rcs/%n",
        inprogress_path = "./rcs/running",
        ttyrec_path = "./rcs/ttyrecs",
        socket_path = "./rcs",
        client_path = "./webserver/game_data/")),
    ("sprint-web-trunk", dict(
        name = "Sprint trunk",
        crawl_binary = "./crawl",
        rcfile_path = "./rcs/",
        macro_path = "./rcs/",
        morgue_path = "./rcs/%n",
        inprogress_path = "./rcs/running",
        ttyrec_path = "./rcs/ttyrecs",
        socket_path = "./rcs",
        client_path = "./webserver/game_data/",
        options = ["-sprint"])),
    ("zd-web-trunk", dict(
        name = "Zot Defense trunk",
        crawl_binary = "./crawl",
        rcfile_path = "./rcs/",
        macro_path = "./rcs/",
        morgue_path = "./rcs/%n",
        inprogress_path = "./rcs/running",
        ttyrec_path = "./rcs/ttyrecs",
        socket_path = "./rcs",
        client_path = "./webserver/game_data/",
        options = ["-zotdef"])),
    ("tut-web-trunk", dict(
        name = "Tutorial trunk",
        crawl_binary = "./crawl",
        rcfile_path = "./rcs/",
        macro_path = "./rcs/",
        morgue_path = "./rcs/%n",
        inprogress_path = "./rcs/running",
        ttyrec_path = "./rcs/ttyrecs",
        socket_path = "./rcs",
        client_path = "./webserver/game_data/",
        options = ["-tutorial"])),
])

dgl_status_file = "./rcs/status"

# Set to None not to read milestones
milestone_file = "./milestones"

status_file_update_rate = 5

recording_term_size = (80, 24)

max_connections = 100

# Script to initialize a user, e.g. make sure the paths
# and the rc file exist. This is not done by the server
# at the moment.
init_player_program = "/bin/echo"

ssl_options = None # No SSL
#ssl_options = {
#    "certfile": "./webserver/localhost.crt",
#    "keyfile": "./webserver/localhost.key"
#}
ssl_address = ""
ssl_port = 8081

connection_timeout = 600
max_idle_time = 5 * 60 * 60

# Seconds until stale HTTP connections are closed
# This needs a patch currently not in mainline tornado.
http_connection_timeout = None

kill_timeout = 10 # Seconds until crawl is killed after HUP is sent

nick_regex = r"^[a-zA-Z0-9]{3,20}$"
max_passwd_length = 20

login_token_lifetime = 7 # Days

uid = None  # If this is not None, the server will setuid to that (numeric) id
gid = None  # after binding its sockets.

umask = None # e.g. 0077

chroot = None

pidfile = None
daemon = False # If true, the server will detach from the session after startup
