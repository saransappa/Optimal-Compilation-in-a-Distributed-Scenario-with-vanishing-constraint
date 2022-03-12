/*

    Refined algorithm implementation with added vanishing constraint

    Updates added to refined.cpp

    An extra constraint named as vanishing constraint is added for optimal memory usage in the servers

*/

#include <algorithm>
#include <chrono>
#include <bits/stdc++.h>
#define pb push_back
#define deb(x) cout<<#x<<" "<<x<<endl; // Debugging function
using namespace std;
using namespace std::chrono;

int nf,nt,ns; // nf --> No.of file to be is_compiled ;  ns --> No.of servers
map<int,vector<int>> dependencies;              // Map to store dependencies of each file with its dependencies as a vector
map<int,int> compile_time;                // Map to store compiling time for each file
map<int,int> replication_time;               // Map to store replication time for each file
map<int,bool> is_compiled;                     // Map to store a boolean whether an file is compiled
map<int,bool> is_replicated;                   // Map to store a boolean whether an file is replicated
map<int,bool> visited;                          // Used in topological sort
stack <int> s;                                  // Used in topological sort
map<int,int> compiled_at_time;              // Map to store the time at which a file is compiled
map<int,int> server_time;                 // Map to store the current server time for each server
map<int,int> compiled_in_server;               // Map to store the server number for each file in which it is compiled 
map<int,bool> is_target;                        // Map to store a boolean to check if a file is a target
map<int,int> deadlines;                         // Map to store deadline for each file
map<int,int> points;                            // Map to store the bonus points for each file
vector<vector<pair<int,pair<int,int>>>> status; // Vector to store a vector of available replicated files
// Pair<file_id,pair<available_time, status>>

int chances = 5; // No.of chances given to a replicated file before deleting

bool contains(vector<pair<int,pair<int,int>>> vec, pair<int,pair<int,int>> elem)
{
    return any_of(vec.begin(), vec.end(), [&](const auto & x){
        return x == elem;
    });
}
// Function to check if all file are compiled
bool check_all_manufactured(int nf,map<int,bool>is_compiled){
    for(int i=0;i<nf;i++){
        if(is_compiled[i]==false)return true;
    }
    return false;
}

// Check presence of a file in the replication list of a server
// Returns index of the file in the replicated files list if available
int present(vector<pair<int,pair<int,int>>> v, int eqp){
    int i = 0;
    for(pair<int,pair<int,int>> p : v){
        if(p.first == eqp)return i;
        i++;
    }
    return -1;
}

// Function to determine the ideal server for file eqp
pair<int,int> ideal_server(int eqp){
    cout<<"Finding ideal server for "<<eqp<<".....\n";
    vector <int> deps_eqp = dependencies[eqp];
    map<int,int> m;
    int ideal_srv = -1;
    int min = INT_MAX;
    for(int i=0;i<ns;i++)m[i] = 0;
    for(int i=0;i<ns;i++){
        int max_wait_time = 0;
        for(int j:deps_eqp){
            if(compiled_in_server[j]!=i){
                int temp,finish_j;
                if(present(status[i],j)>=0){
                    finish_j = compiled_at_time[j];
                    temp = finish_j + replication_time[j];
                }
                else{
                    finish_j = server_time[i];
                    temp = finish_j + replication_time[j];
                }
                max_wait_time = max(max_wait_time,temp);
            }
        }
        m[i] = max(max_wait_time,server_time[i]) ;
        if(min > m[i]){
            min = m[i];
            ideal_srv = i;
        }
    }
    for(int i=0;i<ns;i++)cout<<m[i]<<" ";
    cout<<endl;
    for(int j:deps_eqp){
        int temp,finish_j;
        int check = present(status[ideal_srv],j);
        if(check>=0){
            status[ideal_srv][check].second.second = 0;
        }
        else{
            finish_j = server_time[ideal_srv];
            temp = finish_j + replication_time[j];
            status[ideal_srv].pb(make_pair(j,make_pair(temp,0)));
        }
    }
    return make_pair(min,ideal_srv);
}

// Util function for topological sort
void topologicalSortUtil(int i){
    visited[i] = true;
    vector<int> deps_i = dependencies[i]; // deps_i contains all dependencies of file i
    for(int j: deps_i){
        if(visited[j]==false){
            topologicalSortUtil(j);
        }
    }
    s.push(i);
}

// Topological sort based on bonus points (Custom Topological Sort)
void topologicalSort(){   
    vector<pair<int,int>> equipment_with_bonus;
    for(int i=0;i<nf;i++){
        equipment_with_bonus.push_back(make_pair(points[i],i));
    }
    sort(equipment_with_bonus.begin(),equipment_with_bonus.end());
    for(int i=equipment_with_bonus.size()-1;i>=0;i--){
        int eqp = equipment_with_bonus[i].second;
        if(visited[eqp]==false){
            topologicalSortUtil(eqp);
        }
    }
}

// Function to compile a file and update the required variables
void manufacture(int eqp,int start_time, int ideal_serv){
    cout<<"Compiling of "<<eqp<<" started .."<<endl;
    
    compiled_at_time[eqp]  = start_time + compile_time[eqp];
    server_time[ideal_serv] = compiled_at_time[eqp];
    is_compiled[eqp] = true; 
    compiled_in_server[eqp] = ideal_serv; 
    for(int i=0;i<ns;i++){
        if(i!=ideal_serv){
            status[i].pb(make_pair(eqp,make_pair(compiled_at_time[eqp]+replication_time[eqp], 0))); // Initial status is 0
        }
    }
    vector<pair<int,pair<int,int>>> temp_vec;
    for(pair<int,pair<int,int>> local_status : status[ideal_serv]){
        if(local_status.second.first <= server_time[ideal_serv]){
            for(int rep : dependencies[eqp]){
                if(rep==local_status.first){
                    local_status.second.second = 0;
                }
                else{
                    local_status.second.second++;
                    if(local_status.second.second<chances){
                        temp_vec.pb(local_status);
                    }
                }
            }
        }
    }
    
    status[ideal_serv] = temp_vec;
    cout<<eqp<<" is compiled in "<<ideal_serv<<" at "<<server_time[ideal_serv]<<endl;
}

// Function for refined algorithm implementation
void refined(int file, int manufacturers){
    cout<<"Started compiling ........."<<endl;
    
    for(int i=0;i<file;i++){
        is_compiled[i]=false;
        visited[i] = false;
    }
    topologicalSort();
    stack <int> temp_stack;
    while(!s.empty()){
        temp_stack.push(s.top());
        cout<<s.top()<<endl;
        s.pop();
    }
    
    while(!temp_stack.empty()){
        int equip = temp_stack.top();
        if(is_compiled[equip]){
            temp_stack.pop();
        }
        else{
            pair<int,int> ideal_serv = ideal_server(equip);
            manufacture(equip,ideal_serv.first, ideal_serv.second);
            temp_stack.pop();
        }
    }

    cout<<"All files are compiled ....."<<endl;

}

int main(){
    // Start taking input
    cin>>nf>>nt>>ns;
    int k,c,r,nd;
    for(int i=0;i<nf;i++){
        cin>>k>>c>>r;
        compile_time[k]=c;
        replication_time[k]=r;
        cin>>nd;
        int p;
        for(int j=0;j<nd;j++){
            cin>>p;
            dependencies[i].pb(p);
        }

    }
    int eqp,dead,point;
    for(int j=0;j<nt;j++){
        cin>>eqp>>dead>>point;
        is_target[eqp] = true;
        deadlines[eqp] = dead;
        points[eqp] = point;
    }
    // Stop taking input
    
    // Start timer
    // Initialize empty status vector 
    for(int i=0;i<ns;i++){
        vector<pair<int,pair<int,int>>> temp;
        status.pb(temp); 
    }
    auto start = high_resolution_clock::now();
    refined(nf,ns);
    
    // Stop timer
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    for(int i=0; i<ns;i++){
        cout<<"Server time of "<<i<<" is "<<server_time[i]<<endl;
    }

    // Find the maximum time of server to get total compilation time 
    int max_manuf = INT_MIN;
    for(int i=0;i<nf;i++){
        cout<<"file "<<i<<" is_compiled at time = "<<compiled_at_time[i]<<endl;
        max_manuf= max(max_manuf,compiled_at_time[i]);
    }
    cout<<"Total time for Compiling = "<<max_manuf<<endl;

    // server for each file
    for(int i=0;i<nf;i++){
        cout<<i<<" is_compiled in s"<<compiled_in_server[i]<<endl;
    }

    // Calculate Score
    int score = 0;
    for(int i=0;i<nf;i++){
        if(is_target[i]){
            if(compiled_at_time[i]<=deadlines[i]){
                score+= points[i];                                  // Bonus points
                score+= (deadlines[i] - compiled_at_time[i]);   // Speed points
            }
        }
    }
    cout<<"Score = "<<score<<endl;
    cout << "Time taken = "<<duration.count() <<" microseconds"<< endl;
    return 0;
}