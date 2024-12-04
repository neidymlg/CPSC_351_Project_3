#include <iostream>
#include <vector>
#include <queue>
#include <fstream>

//id number, arrival time, lifetime, full memory, load arrival time
struct Process {
    int id;
    int arrival;
    int lifetime;
    int memory;
    int load_arr;
};

int MEMORY_SIZE;
int PAGE_SIZE;
std::deque<Process> wait_queue;
std::vector<std::pair<int, int>> buffer;

//functions
void print_mm(std::ofstream &outFile);
void print_wq(std::ofstream &outFile);

//for this project we will assume in1.txt file has the processes in order based on arrival time
int main() {
	//values to initalize
	int process_num;
	int page_option;
	std::vector<Process> processes;
	std::deque<Process> exit_queue;
        double turn_around = 0;
	int currentTime = 0;
	
	//input from user
	std::cout << "Memory size> ";
	std::cin >> MEMORY_SIZE;
	
 	std::cout << "Page Size (1: 100, 2: 200, 3: 400)> ";
 	std::cin >> page_option;
	
	switch (page_option) {
      		case 1:
			PAGE_SIZE = 100;
		break;
        	case 2:
			PAGE_SIZE = 200;
            	break;
        	case 3:
            		PAGE_SIZE = 400;
            	break;
        	default:
            	std::cerr << "Invalid option. Defaulting to page size 100." << std::endl;
            	PAGE_SIZE = 100;
            	break;
	}
	
	//if not a multiple then exit 
	if (MEMORY_SIZE % PAGE_SIZE != 0) {
        	std::cerr << "Error: Memory size must be a multiple of page size." << std::endl;
        	return 1;
	}

	// Initialize the buffer and available_memory
        buffer.resize(MEMORY_SIZE / PAGE_SIZE, {-1, -1});
        int available_memory = MEMORY_SIZE;
        
       //Opens file 
	std::ifstream file("in1.txt");
	if (!file) {
	        perror("Error: File cannot be read");
	        return 1;
	}
	
	//reads processses from the file 
	if (file >> process_num) {
		//resize processes based on process_num
		processes.resize(process_num+1);
	        int id, arrival, lifetime, count, memory;
	        while (file >> id >> arrival >> lifetime >> count) {
		            int mem_sum = 0;
		            processes[id].id = id;
		            processes[id].arrival = arrival;
		            processes[id].lifetime = lifetime;
		
			for (int i = 0; i < count; ++i) {
		                if (file >> memory) {
		                    mem_sum += memory;
	               		}
	               }

		processes[id].memory = mem_sum;
        	}
        }
	else {
        	perror("Error: Amount of processes not specified");
		return 1;
	}
	
        
	file.close(); //close file


        //creates file to output all information to, or overwrites file
	std::ofstream outFile("MM_output.txt");
	if (!outFile) {
	        std::cerr << "Error: File cannot be opened for writing" << std::endl;
	        return 1;
	}

        int arrival_index = 1;	
	while (currentTime <= 100000 || !exit_queue.empty()) {
		// Check for process arrivals starting from id 1 
		// if process has arrived, push/show wait_queue
		// else the process has not arrived, and the next while loop will check if that process has arrived
		for (; arrival_index < process_num + 1; arrival_index++) {
			if (processes[arrival_index].arrival <= currentTime) {
				wait_queue.push_back(processes[arrival_index]);
	   			outFile << "Process " << processes[arrival_index].id << " arrived at time " << currentTime << std::endl;
                	    	print_wq(outFile);
                	} else {
                    		break;
                	}
            	}

		// for any process that has exited the wait_queue, it will enter the exit queue to wait for its lifetime to expire
		// if the time it has arrived at + lifetime is now smaller than the current time, it will calculate the total turnaround
		// it will then go through the buffer and reset all instance that it occupied and add in the available memory back
		// it will exit the exit queue
		for (auto it = exit_queue.begin(); it != exit_queue.end();){
			if(it->load_arr + it->lifetime <= currentTime){
				turn_around = turn_around + (currentTime - it->arrival);
            			for (int i = 0; i < MEMORY_SIZE / PAGE_SIZE; i++) {
					if (buffer[i].first == it->id) {
						buffer[i] = {-1, -1};
						available_memory += PAGE_SIZE;
        				}
                		}	
				outFile << "Process " << it->id << " completes at " << currentTime << "\n";
				it = exit_queue.erase(it);
				print_mm(outFile);
                	}
			else{
                		++it;
        	        }
		}
	    
		// For any process that is qaiting in the wait_queue to be loaded
		// if the buffer has sufficient memory, then we will mark the load arrival time, 
		// and start looking for free spaces in the buffer to insert the id and page number, and subtract available memory
		// since it is now waiting to execute and exit, it will be pushed into the exit_queue
		for (auto it = wait_queue.begin(); it != wait_queue.end();){
			if(it->memory <= available_memory){
				it->load_arr = currentTime;
				int buffer_i = 0;
                    		int page_id = 1;
 				while (buffer_i < MEMORY_SIZE / PAGE_SIZE && it->memory > 0) {
                        		if (buffer[buffer_i].first == -1) {
                            			buffer[buffer_i] = {it->id, page_id};
						it->memory -= PAGE_SIZE;
                            			page_id++;
						available_memory -= PAGE_SIZE;
                        		}
                        		buffer_i++;
				}
		    		exit_queue.push_back(*it);
				outFile << "MM moves process " << it->id << " to memory at time " << currentTime << "\n";
				it = wait_queue.erase(it);
				print_mm(outFile);
			}
			else{
				++it;
                	}
		}
		currentTime += 100;
        }
	
	outFile << "Turnaround Time: " << turn_around/process_num << " (" << turn_around << "\\" << process_num << ")\n";
	outFile.close();
	return 0;
}

//function used to print waiting queue
void print_wq(std::ofstream &outFile){
	outFile << "\tInput Queue: [ ";
            for (auto it = wait_queue.begin(); it != wait_queue.end(); it++){
        	outFile << it->id << " ";
    	}
    	
    	outFile << "]\n";
}

//function used to print memory map
void print_mm(std::ofstream &outFile){
	outFile << "\tMemory Map:\n";
	int start_i = 0;
	int end_i = PAGE_SIZE - 1;
	for(int i = 0; i < MEMORY_SIZE / PAGE_SIZE; i++){
		if(buffer[i].first < 1){
			outFile << "\t\t" << start_i << "-" << start_i+PAGE_SIZE-1 << ": Free frame(s)" << "\n";
		}
		else{
			outFile << "\t\t" << start_i << "-" << start_i+PAGE_SIZE-1 << ": Process " << buffer[i].first << ", Page " << buffer[i].second << "\n";
		}
		start_i += PAGE_SIZE;
		end_i = start_i - 1 + PAGE_SIZE;
	}
}