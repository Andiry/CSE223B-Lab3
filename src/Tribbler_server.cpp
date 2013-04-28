// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.


#include <stdio.h>
#include <errno.h>
#include <iostream>
#include "Tribbler.h"
#include "KeyValueStore.h"
#include <transport/TSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace std;
using namespace  ::Tribbler;
using namespace  ::KeyValueStore;

class TribblerHandler : virtual public TribblerIf {
 public:

  TribblerHandler(std::string storageServer, int storageServerPort) {
    // Your initialization goes here
    cout << "Server: " << storageServer << " Port: " << storageServerPort << endl;
    _storageServer = storageServer;
    _storageServerPort = storageServerPort;
    _index = 0;
  }

  TribbleStatus::type CreateUser(const std::string& userid) {
    // Your implementation goes here
    cout << "CreateUser " << userid << endl;
    KVStoreStatus::type bs_create;
    TribbleStatus::type s_create;
    GetResponse bs_as;
    string check_user = userid + "_user";

    bs_as = Get(check_user);
    if (bs_as.status == KVStoreStatus::OK && bs_as.value == "Created") {
	cout << "User " << userid << " aleardy exist" << endl;
	return TribbleStatus::OK;
    }
    bs_create = Put(check_user, "Created", "t_s");
    cout << "CreateUser result " << bs_create << endl;
    if (bs_create != KVStoreStatus::OK) {
    	cout << "Failed to create user: Error code " << bs_create << endl;
    } else {
    	cout << "Create user succeed" << endl;
    }
    s_create = static_cast<TribbleStatus::type>(bs_create);
    return s_create;
  }

  TribbleStatus::type AddSubscription(const std::string& userid, const std::string& subscribeto) {
    // Your implementation goes here
    GetResponse bs_as;
    GetListResponse bs_as_res;
    KVStoreStatus::type bs_atl;
    string user_subscribe = userid;
    string check_user = userid + "_user";
    string check_user_sub = subscribeto + "_user";

    printf("AddSubscription %s, %s\n", userid.c_str(), subscribeto.c_str());
    bs_as = Get(check_user);
    if (bs_as.status != KVStoreStatus::OK || bs_as.value != "Created") {
	cout << "User " << userid << " does not exist!" << endl;
	return TribbleStatus::INVALID_USER;
    }
    bs_as = Get(check_user_sub);
    if (bs_as.status != KVStoreStatus::OK || bs_as.value != "Created") {
	cout << "User " << subscribeto << " does not exist!" << endl;
	return TribbleStatus::INVALID_SUBSCRIBETO;
    }

    user_subscribe += "_sublist";
    bs_atl = AddToList(user_subscribe, subscribeto, "t_s");
    if (bs_atl != KVStoreStatus::OK && bs_atl != KVStoreStatus::EITEMEXISTS) {
	cout << "Subscribe to user " << subscribeto << " failed!" << endl;
	return TribbleStatus::INVALID_SUBSCRIBETO;
    }
    
    bs_as_res = GetList(user_subscribe);
    if (bs_as_res.status == KVStoreStatus::OK && !bs_as_res.values.empty()) {
	cout << userid << " subscribed to ";
	vector<string>::iterator i = bs_as_res.values.begin();
	for (i = bs_as_res.values.begin(); i != bs_as_res.values.end(); ++i)
	    cout << *i << " ";
	cout << endl;
    }
    return TribbleStatus::OK;
  }

  TribbleStatus::type RemoveSubscription(const std::string& userid, const std::string& subscribeto) {
    // Your implementation goes here
    GetResponse bs_as;
    KVStoreStatus::type bs_atl;
    string user_subscribe = userid;
    string check_user = userid + "_user";
    string check_user_sub = subscribeto + "_user";

    printf("RemoveSubscription %s, %s\n", userid.c_str(), subscribeto.c_str());
    bs_as = Get(check_user);
    if (bs_as.status != KVStoreStatus::OK || bs_as.value != "Created") {
	cout << "User " << userid << " does not exist!" << endl;
	return TribbleStatus::INVALID_USER;
    }
    bs_as = Get(check_user_sub);
    if (bs_as.status != KVStoreStatus::OK || bs_as.value != "Created") {
	cout << "User " << subscribeto << " does not exist!" << endl;
	return TribbleStatus::INVALID_SUBSCRIBETO;
    }

    user_subscribe += "_sublist";
    bs_atl = RemoveFromList(user_subscribe, subscribeto, "t_s");
    if (bs_atl == KVStoreStatus::OK) {
	return TribbleStatus::OK;
    } else if (bs_atl == KVStoreStatus::EITEMNOTFOUND) {
	cout << userid << " is not subscribed to " << subscribeto << endl;
	return TribbleStatus::OK;
    } else {
	cout << "Remove subscription failed! " << userid << " " << subscribeto << endl;
    	return TribbleStatus::INVALID_SUBSCRIBETO;
    }
  }

  TribbleStatus::type PostTribble(const std::string& userid, const std::string& tribbleContents) {
    // Your implementation goes here
    GetResponse bs_as;
    KVStoreStatus::type bs_atl;
    struct Tribble tribble;
    string user_tribble = userid;
    time_t lt;
    string tribble_string;
    string timestamp;
    char t[256];
    string check_user = userid + "_user";

    printf("PostTribble %s, %s\n", userid.c_str(), tribbleContents.c_str());
    bs_as = Get(check_user);
    if (bs_as.status != KVStoreStatus::OK || bs_as.value != "Created") {
	cout << "User " << userid << " does not exist!" << endl;
	return TribbleStatus::INVALID_USER;
    }

    lt = time(NULL);
    tribble.userid = userid;
    tribble.contents = tribbleContents;
    tribble.posted.push_back(static_cast<uint64_t>(lt));

    sprintf(t, "%d", _index++);
    timestamp = t;
    tribble_string = "{{" + tribble.userid + "},{" + tribble.contents + "}}";
    cout << tribble_string << endl;
    user_tribble += "_tribbles";

    // Post tribble to the backend server
#if 0
    bs_atl = AddToList(timestamp, tribble_string);
    if (bs_atl != KVStoreStatus::OK) {
	cout << "1 " << bs_atl << endl;
	return TribbleStatus::INVALID_USER;
    }
#endif
    bs_atl = AddToList("ts", tribble_string, timestamp);
    if (bs_atl == KVStoreStatus::OK || bs_atl == KVStoreStatus::EITEMEXISTS) {
	return TribbleStatus::OK;
    }

    cout << "2 " << bs_atl << endl;
    return TribbleStatus::INVALID_USER;
  }

  int process_tribble(struct Tribble* tribble, string tribble_string, string *id) {
    int userid_start = 2;
    unsigned int userid_end;
    int tribble_start, tribble_end;
    int length;
    string strset = "}";
    string userid, tribble_content;

    userid_end = tribble_string.find_first_of(strset);
    if (userid_end == string::npos) {
	cout << "Not find }" << endl;
        return -1;
    }
    userid = tribble_string.substr(userid_start, userid_end - userid_start);

    length = tribble_string.length();

    tribble_start = userid_end + 3;
    tribble_end = length - 2;
    tribble_content = tribble_string.substr(tribble_start, tribble_end - tribble_start);

    tribble->userid = userid;
    tribble->contents = tribble_content;
    *id = userid;
    return 0;

  }

  struct decrease
  {
  	inline bool operator() (const string& s1, const string& s2)
	{
    		return atoi(s1.c_str()) > atoi(s2.c_str());
  	}
  };  

  struct decrease1
  {
  	inline bool operator() (const Tribble& s1, const Tribble& s2)
	{
    		return s1.posted > s2.posted;
  	}
  };  

  void GetTribblesByTimeList(TribbleResponse& _return, const std::vector<string>& timelist) {
    GetResponse ret;
    vector<string>::const_iterator iter;
    string tribble_content;
    struct Tribble tribble;
    string id;

    for (iter = timelist.begin(); iter != timelist.end(); ++iter) {
	ret = Get(*iter);
	if (ret.status == KVStoreStatus::OK) {
	    tribble_content = ret.value;
	    if (!process_tribble(&tribble, tribble_content, &id)) {
		tribble.posted.push_back(atoi((*iter).c_str()));
    	    	_return.tribbles.push_back(tribble);
	    }
	}
    }
  }

    // Your implementation goes here
  void GetTribbles(TribbleResponse& _return, const std::string& userid) {
    // Your implementation goes here
    GetResponse bs_as;
    string user_tribble = userid;
    GetListResponse bs_gt_res, tribbles;
    struct Tribble tribble;
    string id;
    string check_user = userid + "_user";

    printf("GetTribbles\n");
    bs_as = Get(check_user);
    if (bs_as.status != KVStoreStatus::OK || bs_as.value != "Created") {
	cout << "User " << userid << " does not exist!" << endl;
    	_return.status = TribbleStatus::INVALID_USER;
	return;
    }

    user_tribble += "_tribbles";
    bs_gt_res = GetList(user_tribble);
    if (bs_gt_res.status == KVStoreStatus::OK && !bs_gt_res.values.empty()) {
	cout << userid << " Tribbles: ";
	GetTribblesByTimeList(_return, bs_gt_res.values);
    }
    _return.status = TribbleStatus::OK;
  }

  void GetTribblesBySubscription(TribbleResponse& _return, const std::string& userid) {
    // Your implementation goes here
    GetResponse bs_as;
    string user_tribble = userid;
    GetListResponse bs_gt_res;
    SubscriptionResponse user_sublist;
    TribbleResponse tribbles;
    struct Tribble tribble;
    unsigned int size;
    string check_user = userid + "_user";
    string sub_user;
    vector<string> timelist;

    printf("GetTribblesBySubscription\n");
    bs_as = Get(check_user);
    if (bs_as.status != KVStoreStatus::OK || bs_as.value != "Created") {
	cout << "User " << userid << " does not exist!" << endl;
    	_return.status = TribbleStatus::INVALID_USER;
	return;
    }

    GetSubscriptions(user_sublist, userid);
    if (user_sublist.status != TribbleStatus::OK) {
	cout << "GetSubscriptions for user " << userid << " failed." <<endl;
	_return.status = user_sublist.status;
	return;
    }

    size = user_sublist.subscriptions.size();
    for (unsigned int i = 0; i < size; ++i) {
	sub_user = user_sublist.subscriptions[i];
	sub_user += "_tribbles";
        bs_gt_res = GetList(sub_user);
	if (bs_gt_res.status == KVStoreStatus::OK && !bs_gt_res.values.empty()) {
	    timelist.insert(timelist.end(), bs_gt_res.values.begin(),
				bs_gt_res.values.end());
	}
    }

    sort(timelist.begin(), timelist.end(), decrease());
    size = timelist.size();

    if (size > 100) {
        timelist.resize(100);
    }

    GetTribblesByTimeList(_return, timelist);
    _return.status = TribbleStatus::OK;
  }

  void GetSubscriptions(SubscriptionResponse& _return, const std::string& userid) {
    // Your implementation goes here
    printf("GetSubscriptions %s\n", userid.c_str());
    GetResponse bs_as;
    string user_subscribe = userid;
    GetListResponse bs_as_res;
    string check_user = userid + "_user";

    _return.status = TribbleStatus::NOT_IMPLEMENTED;
    bs_as = Get(check_user);
    if (bs_as.status != KVStoreStatus::OK || bs_as.value != "Created") {
	cout << "User " << userid << " does not exist!" << endl;
	return;
    }

    user_subscribe += "_sublist";
    bs_as_res = GetList(user_subscribe);
    if (bs_as_res.status == KVStoreStatus::OK && !bs_as_res.values.empty()) {
	_return.subscriptions = bs_as_res.values;
	_return.status = TribbleStatus::OK;
	return;
    }
    _return.status = TribbleStatus::INVALID_SUBSCRIBETO;
  }

  // Functions from interacting with the storage RPC server
  KVStoreStatus::type AddToList(std::string key, std::string value, std::string clientid) {
    boost::shared_ptr<TSocket> socket(new TSocket(_storageServer, _storageServerPort));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    KeyValueStoreClient kv_client(protocol);
    // Making the RPC Call
    KVStoreStatus::type st;
    transport->open();
    st = kv_client.AddToList(key, value, clientid);
    transport->close();
    return st;
  }

  KVStoreStatus::type RemoveFromList(std::string key, std::string value, std::string clientid) {
    // Making the RPC Call to the Storage server
    boost::shared_ptr<TSocket> socket(new TSocket(_storageServer, _storageServerPort));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    KeyValueStoreClient client(protocol);
    KVStoreStatus::type st;
    transport->open();
    st = client.RemoveFromList(key, value, clientid);
    transport->close();
    return st;
  }

  KeyValueStore::GetListResponse GetList(std::string key) {
    // Making the RPC Call to the Storage server
    KeyValueStore::GetListResponse response;
    boost::shared_ptr<TSocket> socket(new TSocket(_storageServer, _storageServerPort));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    KeyValueStoreClient client(protocol);
    transport->open();
    client.GetList(response, key);
    transport->close();
    return response;
  }

  KVStoreStatus::type Put(std::string key, std::string value, std::string clientid) {
    // Making the RPC Call to the Storage server
    boost::shared_ptr<TSocket> socket(new TSocket(_storageServer, _storageServerPort));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    KeyValueStoreClient client(protocol);
    KVStoreStatus::type st;
    transport->open();
    st = client.Put(key, value, clientid);
    transport->close();
    return st;
  }

  KeyValueStore::GetResponse Get(std::string key) {
    KeyValueStore::GetResponse response;
    // Making the RPC Call to the Storage server
    boost::shared_ptr<TSocket> socket(new TSocket(_storageServer, _storageServerPort));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    KeyValueStoreClient client(protocol);
    transport->open();
    client.Get(response, key);
    transport->close();
    return response;
  }

 private:
  std::string _storageServer;
  int _storageServerPort;
  int _index;
};

int main(int argc, char **argv) {
  if (argc != 4) {
    cerr << "Usage: " << argv[0] << " <storageServerIP> <storageServerPort> <tribbleServerPort>" << endl;
    exit(0);
  }
  std::string storageServer = std::string(argv[1]);
  int storageServerPort = atoi(argv[2]);
  int tribblerPort = atoi(argv[3]);

  shared_ptr<TribblerHandler> handler(new TribblerHandler(storageServer, storageServerPort));
  shared_ptr<TProcessor> processor(new TribblerProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(tribblerPort));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  cout << "Starting Tribbler Server" << endl;
  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}
