#ifndef LIGHTNING_FLASH_H
#define LIGHTNING_FLASH_H

#include"Actor/Actor.h"

class LightningFlash :public Actor
{

public:

	LightningFlash(IWorld* world = nullptr, const GSvector3& position = { 0.0f,0.0f,0.0f }, const int delete_num = -1);
	void update(float delta_time)override;
	void draw()const override;
	void  draw_transparent()const override;
	virtual void react(Actor& other)override;

private:
	int lf_state_{ 0 };
	float flash_timer_{ 0.0f };
	//どちら側を光らせるかのフラグ
	bool player1_side_; 
	bool player2_side_;

	GSvector3 origin_position_{ 0.0f,0.0f,0.0f };//他アクターが触れる前のライトニングフラッシュの座標
	GSvector2 image_size_{ 0.0f,0.0f };

private:
	enum LFState
	{
		LF_Default,
		LF_Flash,
		LF_Wait,
	};
};

#endif