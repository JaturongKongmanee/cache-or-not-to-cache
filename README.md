# cache-or-not-to-cache
## Introduction
To simulate a simple IR-based cache invalidation model using [CSIM](http://www.mesquite.com/).
 * Reference paper: [A Scalable Low-Latency Cache Invalidation Strategy for
Mobile Environments](http://www.cs.columbia.edu/~danr/courses/6762/Summer03/week13/mobile-cache.pdf)


 ## Getting Started
 ### Prerequisites
  * install [Code::Blocks](http://www.codeblocks.org/) and [C Programming Language](https://www.geeksforgeeks.org/c-language-set-1-introduction/)
  * install [CSIM](http://www.mesquite.com/)

### System Parameters
|  |  |
| --- | --- |
| Number of clients | 100 |
| Database size | 1000 items |
| Data item size | 1024 bytes |
| Broadcast interval (L) | 20 seconds |
| Broadcast bandwidth | 10000 bits/s |
| Cache size | 50 to 300 items |
| Mean query generate time (T_query) | 25s to 300s |
| Mean update arrival time (T_update) | 20s to 10000s |
| Hot data items | 1 to 50 |
| Cold data items | remainder of DB |
| Hot data access probability | 0.8 |
| Hot data update probability | 0.33|

### Simulation Model and Functions
| | **Main Functions** | **Utility Functions** |
| --- | --- | --- |
| **Server** | void update_data_items() | int is_duplicated() |
|        | void invalidation_report() | int get_list_size() |
|        | void receive_request() | void clear_list() |
|        |  | int get_list_size() |
| **Client** | void generate_query() |  int is_cached() |
|        | void receive_ir() | lnt get_oldest_valid() |
|        |  | int is_cache_full() |
|        |  | int get_oldest_invalid() |

 ### Compilation & Run
 ```python
 csim64.gcc project.c -o project
 ./project
 ```
 To show the simulation is running correctly in the below section, I simplify the parameters according to:
 * Number of clients is **3** 
 * Mean update arrival time (T_update) is **20s**
 * Mean query generate time (T_query) is **100s**
 * Simulation time is **50000**
 * Cache size is **100** items
 
  ### Run Simulation
  
* Specify T_update and T_query.
```python
Enter Mean update arrival time (T_update) in seconds:
20
You've entered T_update: 20
Enter Mean query generate time (T_query) in seconds:
100
You've entered T_query: 100
```
 * Example of statistical results.
 ```python
Enter Mean update arrival time (T_update) in seconds: 20
Enter Mean query generate time (T_query) in seconds: 100
#Cache hit: 831 ------ #Cache miss: 1179
#Cache hit ratio 0.41
Query delay (seconds):        5.353
#Queries served per interval: raw 431, avg: 0.17240
```

#### The section below shows the processes of simulation step by step.
* Clients and server are generated properly
```python
Calling server
Calling client 1
Calling client 2
Calling client 3
Server is generated
Setting up a database ...
Client 1 is generated
Node 1, cache address is 6527296
Client 2 is generated
Node 2, cache address is 6530496
Client 3 is generated
Node 3, cache address is 6533696
```

* Updating data items in the database at server is running properly
```python
Updating data items ...
Updating COLD DATA ITEM at index 559, updated time  1.076
Updating COLD DATA ITEM at index 76, updated time  6.416
.
.
.
Updating HOT DATA ITEM at index 1, updated time 449.938
Updating HOT DATA ITEM at index 45, updated time 450.169
Updating COLD DATA ITEM at index 396, updated time 454.946

```
* Creating IR and Broadcasting IR to all clients are running properly
```python
Creating IR ...
Getting IR LIST size ... 
Creating an IR ... with size 2 at address 6537696
Testing IR ... data item id 559, updated time  1.076, IR size 2
Testing IR ... data item id 76, updated time  6.416, IR size 2
Broadcasting IR to node 1 
Broadcasting IR to node 2 
Broadcasting IR to node 3 
IR counter is set to 0
Getting L_bcast LIST size ... 
Node 1 address 6527296, receives IR size 2
IR id 559
IR id 76
Node 2 address 6530496, receives IR size 2
IR id 559
IR id 76
Node 3 address 6533696, receives IR size 2
IR id 559
IR id 76
```

* Invalidating cache items at the client.
```python
Deallocating IR's memory ... 
Getting IR LIST size ... 
Getting L_bcast LIST size ... 
Creating an IR ... with size 2 at address 6537696
Testing IR ... data item id 123, updated time 361.445, IR size 2
Testing IR ... data item id 21, updated time 367.377, IR size 2
Broadcasting IR to node 1 
Broadcasting IR to node 2 
Broadcasting IR to node 3 
IR counter is set to 0
Node 1 address 6527296, receives IR size 2
IR id 123
IR id 21
Invalidating cache id 21, valid 0, updated time  0.000 ... at node 1
Node 2 address 6530496, receives IR size 2
IR id 123
IR id 21
Invalidating cache id 21, valid 0, updated time  0.000 ... at node 2
Node 3 address 6533696, receives IR size 2
IR id 123
IR id 21
Invalidating cache id 21, valid 0, updated time  0.000 ... at node 3
Deallocating IR's memory ... 
```

* Client: generating the query request if cache miss.
* Server: receiving the query request, check if the query request is duplicated.
```python
node 3 CACHE MISS id 28
Client 3 is GENERATING QUERY request at HOT data... with id 28 at 455.114
Id 28 is NOT duplicated
Server RECEIVES QUERY request id 28
node 2 CACHE MISS id 188
Client 2 is GENERATING QUERY request at COLD data... with id 188 at 457.300
Id 188 is NOT duplicated
Server RECEIVES QUERY request id 188
Getting L_bcast LIST size ... 
Creating an L_bcast ... with size 2 at address 6524000
Broadcasting DATA ITEM to node 1 
Broadcasting DATA ITEM to node 2 
Broadcasting DATA ITEM to node 3 
Testing DATA ITEM ... id 28, updated time  0.000, IR size 2
Testing DATA ITEM ... id 188, updated time  0.000, IR size 2
QUERY counter is set to 0
Node 1 address 6527296, receives DATA ITEM size 2
DATA ITEM id 28
NEW DATA ITEM is cached: id 28, valid 1, last updated time  0.000, last accessed time  0.000
DATA ITEM id 188
NEW DATA ITEM is cached: id 188, valid 1, last updated time  0.000, last accessed time  0.000
Node 3 address 6533696, receives DATA ITEM size 2
DATA ITEM id 28
NEW DATA ITEM is cached: id 28, valid 1, last updated time  0.000, last accessed time  0.000
DATA ITEM id 188
NEW DATA ITEM is cached: id 188, valid 1, last updated time  0.000, last accessed time  0.000
Node 2 address 6530496, receives DATA ITEM size 2
DATA ITEM id 28
NEW DATA ITEM is cached: id 28, valid 1, last updated time  0.000, last accessed time  0.000
DATA ITEM id 188
NEW DATA ITEM is cached: id 188, valid 1, last updated time  0.000, last accessed time  0.000
```

* Answering the query if cache hit
```python
node 3 CACHE HIT id 28 ---- generated query id 28
.
.
node 1 CACHE HIT id 20 ---- generated query id 20
.
.
node 1 CACHE HIT id 23 ---- generated query id 23
node 2 CACHE HIT id 47 ---- generated query id 47
```

* Replacing the **oldest invalid data items**
```python
Getting L_bcast LIST size ... 
Creating an L_bcast ... with size 2 at address 6524000
Broadcasting DATA ITEM to node 1 
Broadcasting DATA ITEM to node 2 
Broadcasting DATA ITEM to node 3 
Testing DATA ITEM ... id 524, updated time  0.000, IR size 2
Testing DATA ITEM ... id 382, updated time 16268.625, IR size 2
QUERY counter is set to 0
Node 1 address 6527296, receives DATA ITEM size 2
DATA ITEM id 524
node 1 cache is full
OLDEST INVALID data item id 668 idx 15 --- is replaced by new data item 524
DATA ITEM id 382
node 1 cache is full
OLDEST VVVVALID data item id 644 idx 95 --- is replaced by new data item 382
Node 2 address 6530496, receives DATA ITEM size 2
DATA ITEM id 524
node 2 cache is full
OLDEST INVALID data item id 668 idx 15 --- is replaced by new data item 524
DATA ITEM id 382
node 2 cache is full
OLDEST VVVVALID data item id 644 idx 95 --- is replaced by new data item 382
Node 3 address 6533696, receives DATA ITEM size 2
DATA ITEM id 524
node 3 cache is full
OLDEST INVALID data item id 668 idx 15 --- is replaced by new data item 524
DATA ITEM id 382
node 3 cache is full
OLDEST VVVVALID data item id 644 idx 95 --- is replaced by new data item 382
```

* Replacing the **oldest valid items** by **LRU**
```python
Getting L_bcast LIST size ... 
Creating an L_bcast ... with size 1 at address 6524000
Broadcasting DATA ITEM to node 1 
Broadcasting DATA ITEM to node 2 
Broadcasting DATA ITEM to node 3 
Testing DATA ITEM ... id 26, updated time 22207.234, IR size 1
QUERY counter is set to 0
Node 1 address 6527296, receives DATA ITEM size 1
DATA ITEM id 26
node 1 cache is full
OLDEST VVVVALID data item id 18 idx 98 --- is replaced by new data item 26
Node 3 address 6533696, receives DATA ITEM size 1
DATA ITEM id 26
node 3 cache is full
CACHED DATA ITEM is updated: id 26, valid 1, last updated time 22207.234, last accessed time 20540.735
node 3 cache is full
node 3 CACHE HIT id 9 ---- generated query id 9
Node 2 address 6530496, receives DATA ITEM size 1
DATA ITEM id 26
node 2 cache is full
OLDEST VVVVALID data item id 18 idx 97 --- is replaced by new data item 26
```

* Checking if the cache is full before collecting statistical data
```python
node 2 cache is full
node 2 CACHE HIT id 8 ---- generated query id 8
.
.
'
node 1 cache is full
node 1 CACHE HIT id 44 ---- generated query id 44
```



 ### Simulation Results
 #### ***The Cache Hit Ratio***
  ![cache_hit_ratio_diff_cache_size](https://github.com/JaturongKongmanee/cache-or-not-to-cache/blob/master/images/cache_hit_ratio_diff_cache_size.png) 
  ![cache_hit_ratio_diff_node](https://github.com/JaturongKongmanee/cache-or-not-to-cache/blob/master/images/cache_hit_ratio_diff_node.png) 
  
  
 #### ***The Query Delay***
 ![query_delay_query_generate_time](https://github.com/JaturongKongmanee/cache-or-not-to-cache/blob/master/images/query_delay_query_generate_time.png) 
  ![query_delay_update](https://github.com/JaturongKongmanee/cache-or-not-to-cache/blob/master/images/query_delay_update.png)
  

 
 #### ***The Number of Queries Served Per IR Interval***
 ![queies_served_per_IR_interval](https://github.com/JaturongKongmanee/cache-or-not-to-cache/blob/master/images/queies_served_per_IR_interval.png) 
  


 ## Author
  * **Jaturong Kongmanee** - [jaturongkongmanee.github.io/jk/](https://jaturongkongmanee.github.io/jk/)
  
 ## Acknowledgments
  * **Assistant Professor [Sunho Lim](http://www.myweb.ttu.edu/slim/), Ph.D.**
  * Discussion with **[Chetan Karahalli Srinivasa](https://github.com/kschetan25)**


## References
- [CSIM - Official Documents](http://www.mesquite.com/documentation)
- [CSIM - Michigan State University](http://www.cse.msu.edu/~cse808/CSIM_Notes03/cse808/)
- [CSIM - University of Pittsburgh](http://www.pitt.edu/~dtipper/2120/CSIM_tutorial.pdf)
- [CSIM - The Chinese University of Hong Kong](http://www.cse.cuhk.edu.hk/~cslui/CSIM19/index.html)
- [CSIM - U of T](http://www.cs.toronto.edu/~iq/csc354s/)
- [VIM Tutorial](http://www.cse.msu.edu/~cse420/Tutorials/VIM/vim.tutorial)
- [Computer Architecture](https://www.cse.msu.edu/~cse420/)
- [Arrays in C](https://www.cs.swarthmore.edu/~newhall/unixhelp/C_arrays.html)
- [Organizing information in README.md with tables](https://help.github.com/en/articles/organizing-information-with-tables)
- [Markdown Cheatsheet](https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet)
