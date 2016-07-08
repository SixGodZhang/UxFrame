#include <UxFrame.hpp>
#include "Resource.h"
#include "HttpDownloader.h"
#include <sstream>

#define CURL_STATICLIB
#include <curl/curl.h>

class MyWindow : public Ux::Window
{
public:
	MyWindow(UINT id) : Ux::Window(id)
	{
		this->initComponents();
	}

	virtual ~MyWindow()
	{
	}

	void initComponents()
	{
		lblUrl_ = Ux::createLabel();
		lblUrl_->text(L"显示URL");
		lblUrl_->fontColor(Gdiplus::Color(255, 0, 0));
		lblUrl_->fontSize(12.0f);
		lblUrl_->x(20);
		lblUrl_->y(10);
		this->addComponent(lblUrl_);

		lblProgress_ = Ux::createLabel();
		lblProgress_->text(L"显示进度数值");
		lblProgress_->fontColor(Gdiplus::Color(255, 0, 0));
		lblProgress_->fontSize(12.0f);
		lblProgress_->x(20);
		lblProgress_->y(60);
		this->addComponent(lblProgress_);

		progressBar_ = Ux::createProgressBar(IDR_PROGRESS_FG, IDR_PROGRESS_BG);
		progressBar_->x(20);
		progressBar_->y(150);
		progressBar_->percent(0.0f);
		this->addComponent(progressBar_);

		tmr1_ = Ux::createTimer(1.0f, [this] {
			this->onTick();
		});

		Ux::ButtonPtr btn = Ux::createButton(IDR_BUTTON_NORMAL,
			IDR_BUTTON_DOWN, IDR_BUTTON_OVER, IDR_BUTTON_DISABLE);
		btn->x(200);
		btn->y(300);
		btn->setClickFunc([this](Ux::Button *self) {
			this->start();
			self->disable(true);
		});
		this->addComponent(btn);
	}

	void onTick()
	{
		std::wstringstream wss;
		auto state = download_.getState();
		if (state == HttpDownloader::kDownloading)
		{
			double percent = download_.getDownloadingPercent();
			progressBar_->percent(percent);
			wss << L"当前进度：" << int(percent*100) << "%";
			lblProgress_->text(wss.str());
		}
		else if (state == HttpDownloader::kError)
		{
			wss << L"Error: ";
			lblProgress_->text(wss.str());
			tmr1_->stop();
		}
		else if (state == HttpDownloader::kDone)
		{
			lblProgress_->text(L"Finished!");
			tmr1_->stop();
		}	
	}

	void setUrl(const std::string &url)
	{
		url_ = url;
		download_.setUrl(url.c_str());

		auto pos = url.rfind("/");
		std::string filename = url.substr(pos+1);
		download_.setLocalFilename(filename.c_str());
	}

	void start()
	{
		download_.go();
		tmr1_->start();
	}

private:
	Ux::TimerPtr tmr1_;
	Ux::LabelPtr lblUrl_;
	Ux::LabelPtr lblProgress_;
	Ux::ProgressBarPtr progressBar_;

	std::string url_;
	HttpDownloader download_;
};

int main(int argc, char* argv[])
{
	curl_global_init(CURL_GLOBAL_ALL);

	auto wnd = std::dynamic_pointer_cast<MyWindow>(Ux::createWindow<MyWindow>(IDR_BGIMAGE));
	wnd->setUrl(argv[argc-1]);
	wnd->show();

	curl_global_cleanup();
	return 0;
}