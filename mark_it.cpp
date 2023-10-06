#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

std::vector<std::string> parse_string_by_char(std::string in, char ch){
	std::vector<std::string> out;
    std::stringstream s_stream(in); 
    while(s_stream.good()) {
      std::string substr;
      getline(s_stream, substr, ch); //get first string delimited by char
      if ((substr.size() > 0)){
          out.push_back(substr);
	  }
    }
    return out;
}



int main(){
	std::ifstream log_file;
	std::string file_name = "log.txt"; 
	log_file.open(file_name);
	if (log_file.is_open()){
		std::string in_line = "";
		int n_points = 0;
		int n_good = 0;
		int w = 95;
		while ( getline(log_file,in_line)){
			//std::cout<<in_line<<" "<<std::endl;
			std::vector<std::string> parsed_in_line = parse_string_by_char(in_line,' ');
			int time = stoi(parsed_in_line.at(0));
			int xs = stoi(parsed_in_line.at(1));
			int ys = stoi(parsed_in_line.at(2));
			int xa = stoi(parsed_in_line.at(3));
			int ya = stoi(parsed_in_line.at(4));
			if ((time > 60) && (ys < 900)){
				// dayteime and after initial period
				n_points++;
			    bool tracked = ((abs(xs-xa)<w/2) &&  (abs(ys-ya)<w/2)); 
				if (tracked) n_good++;
			    std::cout<<" time="<<time<<" ys="<<ys<<" tr="<<tracked;
			    std::cout<<" npoints="<<n_points<<"  ng="<<n_good<<std::endl;
			} 
			
		}
		std::cout<<std::endl;
		std::cout<<" n_good ="<<n_good<<" n_points="<<n_points<<std::endl;
	} else {
		std::cout<<" Can not open "<<file_name<<std::endl;
	}
}
