This tool allows you to generate sprint burn charts from Jazz without needing to manually copy the data from Jazz into Excel.

A snapshot is saved into the data.xml file every day the tool is run.

The chart can be saved to a file or copied to the clipboard by using the buttons on the window title bar.

# Known Issues

1. Currently the team name is "Acquisition" and you must modify the code to generate charts for other teams.

2. If you run the tool on a weekend, or forget to run the tool during the week, you will need to edit data.xml to fix the missing or extra values. The mapping from snapshot times to sprint days is basically just take one snapshot from each day the tool was run and assume those weren't weekends.

3. When you change from one sprint to the next, data.xml must be deleted to remove the entries from the previous sprint.