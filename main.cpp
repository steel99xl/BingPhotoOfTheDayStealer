#include <iostream>
#include <vector>
#include <curl/curl.h>
#include <fstream>
#include <cstring>

typedef struct stoleimage{
	enum States {NORMAL, VERBOSE, LINK, PERVIEW};
	bool HasUrl = false;
	std::string PageBuffer = "";
	std::string Url = "";
	std::string FileName = "";
	States Option = NORMAL;
	
	operator bool(){return HasUrl;}
} StolenImage;

const char *BingLink = "https://www.bing.com/";
std::string Preview = "xdg-open ";

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

	curl = curl_easy_init();

	if(curl){
		if(ImageToSteal){
			curl_easy_setopt(curl, CURLOPT_URL, ImageToSteal.Url.c_str());
		} else {
			curl_easy_setopt(curl, CURLOPT_URL, BingLink);
		}
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "photothife-agent/1.0");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
         	// Passed Provided string to the callback function noted above
    	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
 
         	// This makes the curl connection
        curl_easy_perform(curl);


	}
	curl_easy_cleanup(curl);
	return data;
}

std::string BingPageToWallPaper(StolenImage ImageToSteal){
	std::string Output; // This is will be the url to the wallpaper

	std::size_t StartPos = ImageToSteal.PageBuffer.find("link rel=\"preload\" href=\"")+26;

	std::size_t EndPos = ImageToSteal.PageBuffer.find("&", StartPos) -4;

	if(ImageToSteal){
		StartPos = ImageToSteal.PageBuffer.find(".", StartPos) + 1;
		Output = ImageToSteal.PageBuffer.substr(StartPos, EndPos-StartPos) + "jpg";
		if(ImageToSteal.Option == stoleimage::VERBOSE){
			std::cout << Output << std::endl;
		}
	} else {
		Output = BingLink + ImageToSteal.PageBuffer.substr(StartPos, EndPos-StartPos) + "jpg";
		if(ImageToSteal.Option == stoleimage::VERBOSE){
			std::cout << Output << std::endl;
		}
	}
	

	return Output;


}

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

int main(int argc, char **argv){
	std::cout << "BingImageStealer V1.1" << std::endl;

	StolenImage ImageToSteal;


	if(argc >= 2){
		if(argv[1][0] == '-'){
			switch(argv[1][1]){
				case 'v' :
					ImageToSteal.Option = stoleimage::VERBOSE;

					break;
				case 'l' :
					ImageToSteal.Option = stoleimage::LINK;

					break;
				case 'p' :
					ImageToSteal.Option = stoleimage::PERVIEW;

					break;
				default :
					std::cout << """./BingPhotoOfTheDay | Steals Bings photo of the day, saves in running dir\n\r./BingPhotoOfTheDay -v | Verbose mode\n\r./BingPhotoOfTheDay -l | Link only mode, only prints link to image\n\r./BingPhotoOfTheDay -p | Preview image of the day\n\r./BingPhotoOfTheDay -h | This Screen...""" << std::endl;
					return 1;
					break;
			}

		}
	}	

	if(ImageToSteal.Option == stoleimage::VERBOSE){
			std::cout << "Connecting to " << BingLink <<  std::endl;
	}

	ImageToSteal.PageBuffer = BingGetter(ImageToSteal);

	ImageToSteal.Url = BingPageToWallPaper(ImageToSteal);
	ImageToSteal.HasUrl = true;
	ImageToSteal.FileName = BingPageToWallPaper(ImageToSteal);

	if(ImageToSteal.Option == stoleimage::LINK){
		std::cout << ImageToSteal.Url << std::endl;
		return 0;
	}

	if(ImageToSteal.Option == stoleimage::PERVIEW){
		std::string TMP = Preview + ImageToSteal.Url;
		system(TMP.c_str());
		return 0;
	}

	if(ImageToSteal.Option == stoleimage::VERBOSE){
		std::cout << ImageToSteal.FileName << std::endl;
		return 0;
	}

	//ImageToSteal.FileName = "UwUStolen.jpg";
	if(ImageToSteal.Option == stoleimage::NORMAL || ImageToSteal.Option == stoleimage::VERBOSE){

		ImageToSteal.PageBuffer = BingGetter(ImageToSteal);
		SaveStolenImage(ImageToSteal);
		std::cout << "An image should have been downloaded" << std::endl;
	}



	return 0;
}
