#ifndef PLAYER_H_
#define PLAYER_H_

#include"Actor/Actor.h"
#include"Util/Util.h"
#include<vector>
using namespace std;

//プライヤークラス
class Player :public Actor
{
public:

public:
	//コンストラクタ
	Player(IWorld* world, const GSvector3& position, const std::string& name, const std::string& tag);

	//コンストラクタ
	Player(IWorld* world, const GSvector3& position, const std::string& name, const std::string& tag,const int online_state, const int key_type,const float is_gauge_dec);
	//更新
	void update(float delta_time)override;
	//描画
	void draw()const override;
	//UI用
	void draw_gui()const override;
	//衝突リアクション
	virtual void react(Actor& other)override;

private:

	void ini_player_setting();
	void update_state(float delta_time);
	void controller(vector<GSint>& key, float delta_time);
	void button_push(vector<GSint>& key);//押されたボタンに応じてtruefalse返す
	string creat_packet_data(bool data);//相手プレイヤーに送るやつ作る

	void online_controller();//オンラインの対戦相手動かすやつ
	void creat_data();//送信データつくるやつ

	void magunet_gauge_system();//マグネットゲージのシステム類

	void reset();

private:

	Util util_;
	//今の移動速度
	float current_speed_;
	//加速率
	float acceleration_;
	//減速率
	float deceleration_;
	//ブレイク回復タイマー
	float recovery_timer_;
	//エフェクトタイマー
	float magnet_effect_timer_;
	//操作キー保存箇所
	vector<GSint>key_code_;
	//コントローラーのスティック
	GSvector2 stic_{ 0.0f,0.0f };
	//ノックバックの速度
	GSvector3 knockback_velocity_;
	//減速
	float knockback_decay_;
	//初期位置
	GSvector3 origin_position_;
	//出現エフェクトの色
	GScolor spown_color_;
	//出現演出時のアルファ
	float spown_alpha_;
	//出現フラグ
	int spown_flag_{ 0 };
	//出現演出タイマー
	float player_start_timer_;
	//
	bool initialized_game_;
	//動き出しフラグ
	bool move_flag_;
	//Breakエフェクトタイマー
	float break_timer_{ 0.0f };
	//Breakエフェクトカウント
	int break_effect_count_{ 0 };
	//
	bool break_effect_played_[3]{ false, false, false };

	//回復速度
	float recovery_magnet_;
	//消費速度
	float decrease_magnet_;
	//引き寄せ可能か
	bool magnet_frag_;
	//ゲージヒールしたか
	bool is_add_magnet_gauge_{ false };
	//
	int prev_hp_stage_ = -1;
	//入力のキー変更
	int key_type_{ 0 };
	//ゲージを減らさない
	float is_gauge_dec_{ 0 };

	//操作一括で管理するやつ
	vector<bool> button_manager_;
	vector<bool> ini_button_manager_;
	vector<bool> before_button_manager_;
	//それのオンライン用
	vector<int> button_manager_client_;
	vector<int> ini_button_manager_client_;
	//レバガチャクールタイム
	vector<float>button_cool_time_manager_;

	string packet_data_;//クライアントへデータ送る
	int online_state_{ 0 };//オフラインー＞０　オンライン自機ー＞１　オンライン敵機ー＞２

	enum GatData {

		GD_PosX = 5,
		GD_PosY,
		GD_Hp,
		GD_Magunet_Gauge,

	};

};

#endif