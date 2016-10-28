== sketch for HackATon 2016, project IdeaBoard ==

== Description ==
Very accessible, very easy to use tool for people to share ideas and discuss other people ideas.

== Installation ==
Start "zwamp.exe"
Start Internet Explorer
In the addres bar, type : http://localhost/Hack_A_Ton_2016_02/index.php
In case my PC is running you can use : http://localhost/Hack_A_Ton_2016_02/index.php
You can view / edit the DB using this link : http://localhost/adminer/?username=root&db=ideabord

== Todo ==
- add some design to the whole thing
- add swipe for voting
- add minimal interface when swiping
- implement IOS and Andriod APP
- add security check for all post / req variables used in queries

== Changelog ==
2016.02.15
	- new index.php layout
	- added idea group support
	- ideas can have a title
	- added simple search to filter by title content
	- can delete an idea by setting title or idea to empty string
2016.02.11
	- can comment ideas
	- cand edit comments
	- can edit ideas
2016.02.10
	- added "add.php", "list.php"
	- user can log in and have a session
	- user can add an idea
	- user can see ideas
	- user can vote an idea
2016.02.09
	- experimented with client side only session storring 
2016.02.08
	- created index.php
	