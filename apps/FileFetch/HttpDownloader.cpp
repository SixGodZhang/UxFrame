#include "HttpDownloader.h"
#include <thread>

#define CURL_STATICLIB
#include <curl/curl.h>

//
// HttpDownloaderImpl
//

class HttpDownloaderImpl
{
public:
	HttpDownloaderImpl();
	~HttpDownloaderImpl();

	// phase - kPrepare
	void setUrl(const char* url);
	void setLocalFilename(const char* filename);
	void go();

	// phase - kDownloading
	double getDownloadingPercent() const;

	// phase - kError/kDone
	HttpDownloader::STATE getState() const;
	std::string getErrorMsg() const;

	void reset();

public:
	void doWork();

private:
	static size_t writeCallback(void *contents, size_t size, size_t nmemb, FILE *fp);
	static int progressCallback(HttpDownloaderImpl *impl, double dltotal, double dlnow, double ultotal, double ulnow);

private:
	HttpDownloader::STATE state_;
	std::string emsg_;
	double percent_;
	std::string localFilename_;

	CURL *curl_;
	std::thread thr_;
};

HttpDownloaderImpl::HttpDownloaderImpl():
	state_(HttpDownloader::kPrepare),
	curl_(nullptr),
	percent_(0.0)
{
	reset();
}

HttpDownloaderImpl::~HttpDownloaderImpl()
{
	if (thr_.joinable())
		thr_.join();

	if (curl_)
	{
		curl_easy_cleanup(curl_);
		curl_ = nullptr;
	}
}

void HttpDownloaderImpl::setUrl(const char* url)
{
	if (state_ != HttpDownloader::kPrepare)
		return;

	curl_easy_setopt(curl_, CURLOPT_URL, url);
}

void HttpDownloaderImpl::setLocalFilename(const char* filename)
{
	localFilename_ = filename;
}

static void doCurlWork(HttpDownloaderImpl *impl)
{
	impl->doWork();
}

void HttpDownloaderImpl::go()
{
	if (state_ != HttpDownloader::kPrepare)
		return;

	state_ = HttpDownloader::kDownloading;

	std::thread thr(doCurlWork, this);
	std::swap(thr_, thr);
}

double HttpDownloaderImpl::getDownloadingPercent() const
{
	if (state_ == HttpDownloader::kDone)
		return 1.0;
	else if (state_ == HttpDownloader::kDownloading)
		return percent_;
	else
		return 0.0;
}

HttpDownloader::STATE HttpDownloaderImpl::getState() const
{
	return state_;
}

std::string HttpDownloaderImpl::getErrorMsg() const
{
	return emsg_;
}

void HttpDownloaderImpl::reset()
{
	if (state_ == HttpDownloader::kDownloading)
		return;
	
	state_   = HttpDownloader::kPrepare;
	percent_ = 0.0;

	if (curl_)
	{
		curl_easy_reset(curl_);
	}
	else
	{
		curl_ = curl_easy_init();
		if (!curl_)
		{
			state_ = HttpDownloader::kError;
			emsg_  = "curl_easy_init() failed";
			return;
		}
	}

//	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
//	curl_easy_setopt(curl_, CURLOPT_
}

size_t HttpDownloaderImpl::writeCallback(void *ptr, size_t size, size_t nmemb, FILE *fp)
{
	return fwrite(ptr, size, nmemb, fp);
}

int HttpDownloaderImpl::progressCallback(HttpDownloaderImpl *impl, double dltotal, double dlnow, double ultotal, double ulnow)
{
	if (dltotal > 0.0)
		impl->percent_ = dlnow / dltotal;
	return 0;
}

void HttpDownloaderImpl::doWork()
{
	FILE *fp = fopen(localFilename_.c_str(), "wb");
	if (fp == NULL)
	{
		state_ = HttpDownloader::kError;
		emsg_  = "fopen() failed";
		return;
	}

	CURLcode ret;
	char errmsg[CURL_ERROR_SIZE];

	curl_easy_setopt(curl_, CURLOPT_ERRORBUFFER, errmsg);

	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
	curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curl_, CURLOPT_PROGRESSFUNCTION, progressCallback);
	curl_easy_setopt(curl_, CURLOPT_PROGRESSDATA, this);

	ret = curl_easy_perform(curl_);
	if (ret != CURLE_OK)
	{
		emsg_  = std::string("curl_easy_perform() error: ") + std::string(errmsg);
		state_ = HttpDownloader::kError;
		return;
	}

	fclose(fp);
	state_ = HttpDownloader::kDone;
}

//
// HttpDownloaderImpl
//

HttpDownloader::HttpDownloader()
{
	impl_ = new HttpDownloaderImpl();
}

HttpDownloader::~HttpDownloader()
{
	delete impl_;
	impl_ = nullptr;
}

void HttpDownloader::setUrl(const char* url)
{
	impl_->setUrl(url);
}

void HttpDownloader::setLocalFilename(const char* filename)
{
	impl_->setLocalFilename(filename);
}

void HttpDownloader::go()
{
	impl_->go();
}

double HttpDownloader::getDownloadingPercent() const
{
	return impl_->getDownloadingPercent();
}

HttpDownloader::STATE HttpDownloader::getState() const
{
	return impl_->getState();
}

std::string HttpDownloader::getErrorMsg() const
{
	return impl_->getErrorMsg();
}

void HttpDownloader::reset()
{
	return impl_->reset();
}

//
// Test
//

// nmake -f Makefile
#if defined(MYTEST)
#pragma comment(lib, "libcurl.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wldap32.lib")

int main()
{
	curl_global_init(CURL_GLOBAL_ALL);

	HttpDownloader download;
	download.setUrl("http://cdn2.01234.com.cn/bd/android/DK_HunterV34_5.0.075_sub10026.apk");
	download.setLocalFilename("test.apk");
	download.go();

	while (1)
	{
		if (download.getState() == HttpDownloader::kError)
		{
			printf("error: %s\n", download.getErrorMsg().c_str());
			break;
		}

		if (download.getState() == HttpDownloader::kDone)
		{
			break;
		}

		double percent = download.getDownloadingPercent();
		printf("downloading %f\n", percent);
		::Sleep(100);
	}

	curl_global_cleanup();
	return 0;
}
#endif
