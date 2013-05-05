	CSE223B Lab3
	Jian Xu, A53026658
	jix024@eng.ucsd.edu


********	HOW TO RUN	********

Simply run make, and start the server with the commandline. You can use the tribble_client or web application to test it.

You can start multiple tribble servers and KV servers, and adding new KV servers during running. New KV server does not see posted tribbles, but it sees existing users and corresponding subscription lists. New tribbles posted by other servers will be seen by new KV servers.

When starting a new KV server, you may see connection refused() logs; it's normal.


********	EXPLAINATION      ********

In this part, I will explain how lab3 supports distributed kv servers.

1. How are user names, subscriptions and tribbles stored

Each KV server has following members:

int _id;	// server id
vector < pair<string, int> > _backendServerVector;	// other servers
vector<string> user_list;	// user name list
std::map<string, vector<string> > subscription_list;	// Each user has a subscription list
std::map<string, vector<string> > user_time_list;	// Each user has a tribble timestamp list. Every tribble will have an unique timestamp.
std::map<string, string> time_tribble_list;		// The map of timestamp to tribble contents.
std::map<int, string> time_lock_list;			// Each timestamp can be locked.
int _cur_index;		// current timestamp.		

2. How does a starting server retrieve data from other servers

When a KV server starts, it will fill up the _backendServerVector list. After that, it will retrieve the current data from other servers by calling RetrieveDataFromOtherServers(). 
It will access other servers, get user_list, each user's subscription_list and _cur_index from other servers. This guarantees the server is synchronized with other servers before providing serve.
Note tribbles are not synchronized, so a new KV server can get the user list and subscription list, but no tribble contents.

3. How does a server forward requests to other servers?

When a KV server receives a Put/AddToList/RemoveFromList request from tribbler server, it will forward the requests to all the other servers by calling PropagateToOtherServers().
Other servers will receive this request and process it.

4. How to synchronize timestamp between different servers?

In my implementation, only timestamp of tribbles need to be synchronized between different KV servers. 
When tribbler server sends a tribble to a KV server, The KV servers will do the following steps:

1) Find the latest available time index from _cur_index, which does not have tribble content and is unlocked;
2) Lock the time index by creating a index locked item in time_lock_list;
3) Request locks from all the other KV servers by calling RequestLockFromOtherServers(index). This will ask other servers to lock the index in their time_lock_list.
4) If 3) fails, which means one KV server has the time index already locked. In this case, just unlock all the other servers and return failure.
5) If 3) succeeds, current KV server has exclusive access to current time index. It will set time_tribble_list with index and tribble content, and add the time index to current user's time list;
6) Then KV server askes all the other servers to update their time_tribble_list and user_time_list;
7) Now the KV servers askes all the other servers to release the lock for the time index;
8) The KV server will release its own lock for the time index, and update _cur_index to index.

This request lock - update - release lock guarantees every server will see the same timestamp lists. Also it does not deadlock, becauses if a server finds the lock is helding by other servers, it will release the lock and returns. The only problem is user need to post tribble again, but I think it's OK.


