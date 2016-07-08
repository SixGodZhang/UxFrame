#pragma once

// ==== Usage ====
// HttpDownloader download;
// download.setUrl("https://curl.haxx.se/libcurl/c/getinmemory.html");
// download.go();
// download.getDownloadingPercent();
// download.getState();

#include <string>

class HttpDownloaderImpl;
class HttpDownloader
{
public:
	enum STATE { kPrepare, kDownloading, kError, kDone };

public:
	HttpDownloader();
	~HttpDownloader();

	// phase - kPrepare
	void setUrl(const char* url);
	void setLocalFilename(const char* filename);
	void go();

	// phase - kDownloading
	double getDownloadingPercent() const;

	// phase - kError/kDone
	STATE getState() const;
	std::string getErrorMsg() const;

	void reset();

private:
	HttpDownloaderImpl *impl_;
};
