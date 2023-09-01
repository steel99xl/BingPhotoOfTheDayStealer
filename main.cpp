#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>

#include <curl/curl.h>

#define NAME "BingPhotoOfTheDayStealer"
#define VERSION "V1.2"

// Switch to HTTP vs HTTPS dew to avoid DeepPacketInspection (DPI) issues
const char *BingLink = "http://www.bing.com/";
std::string Preview = "xdg-open ";
unsigned int ImageSizeCheck = 200000; //Image should be no smaller than 200k for 1080p
unsigned int PageSizeCheck = 25; //Minium webpage size of 25 bites for parser
unsigned int TimeOut = 5;


//Base of image from link
typedef struct stolenimage{
	enum States {NORMAL, VERBOSE, LINK, PERVIEW, RENAMEFILE};
	bool HasUrl = false;
	std::string PageBuffer = "";
	std::string Url = "";
	std::string FileName = "";
	States Option = NORMAL;
	unsigned int Count = 0;
	
	operator bool(){return HasUrl;}
} StolenImage;



//Call Back function for curl... based on their example
size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s){
    size_t newLength = size*nmemb;
    try{
        s->append((char*)contents, newLength);
    }
    catch(std::bad_alloc &e){
         // handle memory problem
        return 0;
    }
    return newLength;
}


// Yes it just gets the bing homepage...
std::string BingGetter(StolenImage ImageToSteal){
	std::string data;
	char url[128];
	CURL *curl;
	std::string Agent = NAME;
	Agent = Agent + VERSION;

	curl = curl_easy_init();

	if(curl){
		if(ImageToSteal){
			curl_easy_setopt(curl, CURLOPT_URL, ImageToSteal.Url.c_str());
		} else {
			curl_easy_setopt(curl, CURLOPT_URL, BingLink);
		}
		curl_easy_setopt(curl, CURLOPT_USERAGENT, Agent.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
         	// Passed Provided string to the callback function noted above
    		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
 
         	// This makes the curl connection
        	curl_easy_perform(curl);
	}
	curl_easy_cleanup(curl);
	return data;
}


// WebPage Parser
std::string BingPageToWallPaper(StolenImage ImageToSteal){
	std::string Output; // This is will be the url to the wallpaper

	std::size_t StartPos = ImageToSteal.PageBuffer.find("link rel=\"preload\" href=\"")+26;

	std::size_t EndPos = ImageToSteal.PageBuffer.find("&", StartPos) -4;

	//Gets the image name from the bing page
	if(ImageToSteal){
		StartPos = ImageToSteal.PageBuffer.find(".", StartPos) + 1;
		Output = ImageToSteal.PageBuffer.substr(StartPos, EndPos-StartPos) + "jpg";
		if(ImageToSteal.Option == stolenimage::VERBOSE){
			std::cout << Output << std::endl;
		}
	} else {
		// Make a URL to the image on the bing page
		Output = BingLink + ImageToSteal.PageBuffer.substr(StartPos, EndPos-StartPos) + "jpg";
		if(ImageToSteal.Option == stolenimage::VERBOSE){
			std::cout << Output << std::endl;
		}
	}
	

	return Output;


}


// Write Date to Disk
void SaveStolenImage(StolenImage ImageToSteal){
	std::ofstream Contraband;

	if(strcmp(ImageToSteal.FileName.c_str() , "") == 0){
		Contraband.open(ImageToSteal.Url.c_str(), std::ios::out);
	} else {
		Contraband.open(ImageToSteal.FileName.c_str(), std::ios::out);
	}

	if(Contraband.is_open()){
		Contraband << ImageToSteal.PageBuffer + "\n";
		Contraband.close();
	} else { 
		std::cout << "ERROR could not open FILE for writeing" << std::endl;

	}

}

// Main
int main(int argc, char **argv){
	std::cout << NAME << ' ' << VERSION << std::endl;

	StolenImage ImageToSteal;


	if(argc >= 2){
		if(argv[1][0] == '-'){
			switch(argv[1][1]){
				case 'V' :
					
				case 'v' :
					ImageToSteal.Option = stolenimage::VERBOSE;

					break;
				case 'F' :
				case 'f' :
					//Set Cutsome File Name 
					if(argc >= 3){
						if(argv[2][0] != '\0'){
							ImageToSteal.Option = stolenimage::RENAMEFILE;
							ImageToSteal.FileName = argv[2];
						} else {
							std::cout << " ERROR | '" << argv[2] << "' is not a valid file name..." << std::endl; 
						}
					} else {
						std::cout << "ERROR | File name not provided...Continuing in NORMAL mode" << std::endl;
					}
					break;
				case 'l' :
					ImageToSteal.Option = stolenimage::LINK;

					break;
				case 'p' :
					ImageToSteal.Option = stolenimage::PERVIEW;

					break;
				default :
					std::cout << "./BingPhotoOfTheDay | Steals Bings photo of the day, saves in running dir\n\r./BingPhotoOfTheDay -v | Verbose mode\n\r./BingPhotoOfTheDay -f [FILENAME] | Specify name for the image\n\r./BingPhotoOfTheDay -l | Link only mode, only prints link to image\n\r./BingPhotoOfTheDay -p | Preview image of the day\n\r./BingPhotoOfTheDay -* | This Screen..." << std::endl;
					return 1;
					break;
			}

		}
	}	

	
	// Loop to get Bing webpage 
	do{

		if(ImageToSteal.Count >= TimeOut){
			std::cout << "ERROR | Unable to connect within the TimeOut interval...Check internet connection" << std::endl;
			return 1;
		}
		if(ImageToSteal.Option == stolenimage::VERBOSE){
			std::cout << "Connecting to " << BingLink <<  std::endl;
		}
		ImageToSteal.PageBuffer = BingGetter(ImageToSteal);
		ImageToSteal.Count += 1;	
	
		if(ImageToSteal.PageBuffer.length() < PageSizeCheck && ImageToSteal.Option == stolenimage::VERBOSE){

			std::cout << "ERROR | Did not get vaild bing page... reattempting download" << std::endl;

		}
	}while(ImageToSteal.PageBuffer.length() < PageSizeCheck);
	ImageToSteal.Count = 0;
	//


	// Parse webpage for URL
	ImageToSteal.Url = BingPageToWallPaper(ImageToSteal);
	ImageToSteal.HasUrl = true;
	// Get the name of the image back from the parser
	// NOTE : The page buffer did not get cleared so it reparses the same buffer to get the name
	if(ImageToSteal.Option != stolenimage::RENAMEFILE){
		ImageToSteal.FileName = BingPageToWallPaper(ImageToSteal);
	} else {
		// Change option back to NORMAL once passed name check
		ImageToSteal.Option = stolenimage::NORMAL;
	}

	if(ImageToSteal.Option == stolenimage::LINK){
		std::cout << ImageToSteal.Url << std::endl;
		return 0;
	}

	if(ImageToSteal.Option == stolenimage::PERVIEW){
		std::string TMP = Preview + ImageToSteal.Url;
		system(TMP.c_str());
		return 0;
	}

	if(ImageToSteal.Option == stolenimage::VERBOSE){
		std::cout << ImageToSteal.FileName << std::endl;
	}

	//ImageToSteal.FileName = "UwUStolen.jpg";
	if(ImageToSteal.Option == stolenimage::NORMAL || ImageToSteal.Option == stolenimage::VERBOSE){
		

		// Loop to connect to image link and download
		do{

			if(ImageToSteal.Count >= TimeOut){
				std::cout << "ERROR | Unable to connect within the TimeOut interval...Check internet connection" << std::endl;
				return 1;
			}

			if(ImageToSteal.Option == stolenimage::VERBOSE){
		 	std::cout << "Making connection to steal image..." << std::endl;
			}

			ImageToSteal.PageBuffer = BingGetter(ImageToSteal);
			ImageToSteal.Count += 1;	


			if(ImageToSteal.Option == stolenimage::VERBOSE){
		 	std::cout << "Size of image to save..." << (ImageToSteal.PageBuffer.length() + 1023)/1024 << "KB" << std::endl;
				if(ImageToSteal.PageBuffer.length() < ImageSizeCheck){
					std::cout << "ERROR | Image size to small... reattempting download" << std::endl; 
				}
			}

		}while(ImageToSteal.PageBuffer.length() < ImageSizeCheck);

			ImageToSteal.Count = 0;

		SaveStolenImage(ImageToSteal);
		std::cout << "An image should have been downloaded" << std::endl;


	}


	return 0;
}
