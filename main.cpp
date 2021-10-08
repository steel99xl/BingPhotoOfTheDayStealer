#include <iostream>
#include <vector>
#include <curl/curl.h>
#include <fstream>
#include <cstring>

struct StolenImage{
	bool HasUrl = false;
	std::string PageBuffer = "";
	std::string Url = "";
	std::string FileName = "";
	
	operator bool(){return HasUrl;}
};

const char *BingLink = "https://www.bing.com/";

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
std::string BingGetter(StolenImage ImageToSteal);
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

	std::size_t StartPos = ImageToSteal.PageBuffer.find("background-image: url(")+22;

	std::size_t EndPos = ImageToSteal.PageBuffer.find("&", StartPos);
	if(ImageToSteal){
		StartPos = ImageToSteal.PageBuffer.find(".", StartPos) + 1;
		Output = ImageToSteal.PageBuffer.substr(StartPos, EndPos-StartPos);
	} else {
		Output = BingLink + ImageToSteal.PageBuffer.substr(StartPos, EndPos-StartPos);
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

int main(){
	std::cout << "boop, still under development" << std::endl;

	StolenImage ImageToSteal;
	
	ImageToSteal.PageBuffer = BingGetter(ImageToSteal);

	ImageToSteal.Url = BingPageToWallPaper(ImageToSteal);
	ImageToSteal.HasUrl = true;
	ImageToSteal.FileName = BingPageToWallPaper(ImageToSteal);

	std::cout << ImageToSteal.FileName << std::endl;

	//ImageToSteal.FileName = "UwUStolen.jpg";

	ImageToSteal.PageBuffer = BingGetter(ImageToSteal);

	SaveStolenImage(ImageToSteal);

	std::cout << "An image should have been stolen uwu" << std::endl;


	return 0;
}
