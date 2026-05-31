#ifndef SCENE_RESULT_H_
#define SCENE_RESULT_H_


#include "Scene/IScene.h"
#include"Util/CSVReader.h"

class SceneResult :public IScene {

public:

	SceneResult();

	virtual void start()override;
	virtual void update(float delta_tiime)override;
	virtual void draw()const override;
	virtual void draw_number(int number,const GSvector2& pos,float scale)const;
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
	bool is_game_end_{ false };
	int play_scene_move_effect_{ 0 };
	float scene_move_clear_{ 0.0f };

	float scene_move_change_{ 0.0f };
	float dec_or_not_{ 0.0f };

	enum RSEeffectID {

		RSE_Start,
		RSE_DontPlay,
		RSE_End,

	};

	string before_scene_name_;//直前のシーン名
	string opponent_mode_;
	float game_time_{ 0.0f };
	int score_p1_{ 0 };
	int score_p2_{ 0 };
	int selected_index_{ 0 };
	//コントローラーのスティック
	GSvector2 stic_0_{ 0.0f,0.0f };
	GSvector2 stic_1_{ 0.0f,0.0f };
	string next_scene_;

};

#endif