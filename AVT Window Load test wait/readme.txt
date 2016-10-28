Description :
Experiment how you could implement a DLL that will block an API call until a specific window finishes loading or unloading.
Generic automation issue is that you search for a "component" ( ex: a window ). You find that window, but it is in a loading state ( ex waiting for data from network ). 
If you try to search data in that window you have a chance to not find anything. Automnation script might report as test fail this situation.
This DLL should block until the specific window finishes loading. In this experiment window is getting redrawn. Some components will block window paint until they finish loading.

There should be other ideas to check when a component finished loading. Examples : low CPU usage, no more new components getting added. Main thread is idling instead waiting for events to load data .....
