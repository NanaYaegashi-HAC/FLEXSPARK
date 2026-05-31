#ifndef SCENE_PLAY_H_
#define SCENE_PLAY_H_


#include "Scene/IScene.h"
#include"Util/CSVReader.h"

class ScenePlay :public IScene {

public:

	ScenePlay();

	virtual void start()override;
	virtual void update(float delta_tiime)override;
	virtual void draw()const override;
	virtual bool is_end()const override;
	virtual string next()const override;
	virtual void end()override;
	virtual void before(string& name)override;
	virtual string want_change_next()const override;
	virtual void get_wont_change_scene(string& name)override {}

private:

	CSVReader csv_;
	World world_;
	bool is_end_{ false };
	bool back_title_{ false };
	bool is_game_end_{ false };
	bool is_game_continue_{ false };
	int play_scene_move_effect_{ 0 };
	float scene_move_clear_{ 0.0f };

	float scene_move_change_{ 0.0f };
	float dec_or_not_{ 0.0f };

	enum PSEeffectID {

		PSE_Start,
		PSE_DontPlay,
		PSE_End,

	};

	string before_scene_name_;//íºëOÇÃÉVÅ[Éìñº
	bool is_result_end_{ false };
	float result_timer_{ 0.0f };

	float game_rerode_time_{ 0.0f };
	string next_scene_;

};

#endif