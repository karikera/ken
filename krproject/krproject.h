#pragma once

class Main:public WebCanvas2D
{
public:
	Main() noexcept;
	~Main() noexcept;

	void onDraw() noexcept override;
};
