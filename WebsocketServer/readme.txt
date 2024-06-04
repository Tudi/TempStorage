VSS-UIApp : a windows application project
VSS-Backend : Apache + PHP + MySQL
VSS-DataProcessingShard : Server shard in C++ + Mysql + Websocket

DB has a list of radar devices.
DB has a list of available DPS instances.
Based on some load balancing strategy, the radars are split between DPS shards.
Each DPS will try to connect to radars to monitor it's feed. If an Alert needs to be sent, it will do so and update alert status : created/notified
UI will use backend to create a session. UI can use this session to connect to 1 (or more) DPS servers. 
UI will request a list of DPS servers for the radars it has access to.
UI will subscribe to specific Radar data / Alerts on DPS servers.
DPS server will broadcast radar data / alerts to subscribers.

At the beginning there is probably only 1 instance of everything : DB, Backend, DPS
There should be a load balancer that can assign Backend shard for a UI based on UI-Backend latency.
DPS servers should support redundancy. More than 1 can run in parallel. Only take over functionality of alerts if the main one fails to send heartbeat.