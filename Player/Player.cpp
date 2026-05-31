#include"Actor/Player/Player.h"
#include"IWorld.h"
#include"Actor/Core/Core.h"
#include"Util/UtilNumber.h"
#include"Util/EffectTexturePlayer.h"
#include<gsEffect.h>
#include<iostream>
#include"Actor/Effect/RecoverEffect.h"

const GSvector2 image_size_{ 128.0f,128.0f };
const GSvector2 core_size_{ 64.0f,64.0f };
const float player_speed_{ 5.0f };
const float core_x_plus_{ 20.0f };
const float interval = 6.0f; // 6フレーム間隔
int image_handle_1{ Image_Player_Default };
int image_handle_2{ Image_Player2_Default };
static const float DecRecoveryTime{ 10.0f };//レバガチャで減らせる時間
static const float RecoveryTimeMax{ 300.0f };//蘇生待機時間
static const float RebaGatyaCoolTime{ 5.0f };//レバガチャの一ボタンのクールタイム
static const float pos_offset{ 455.0f };
static const float MagnetRecoverEffectTime{ 60.0f };//引き寄せエフェクトの出す時間
static const GSvector3 MagnetRecoverEffectScale{ 15.0f,15.0f,15.0f };//引き寄せエフェクトのスケール
static const GSvector3 MagnetRecoverEffectAngle{ 0.0f,0.0f,0.0f };//引き寄せエフェクトの回転
static const GSvector3 MagnetRecoverEffectAddPosition{ 0.0f,0.0f,0.0f };//引き寄せエフェクトの座標補間
static const float Magnet_gauge_rate{ 0.5f };

//コンストラクタ
Player::Player(IWorld* world, const GSvector3& position, const std::string& name, const std::string& tag) :
	current_speed_(0.0f),
	acceleration_(0.08f),
	deceleration_(0.1f),
	recovery_timer_(0.0f),
	spown_alpha_(0.0f),
	magnet_effect_timer_(20.0f),
	knockback_velocity_(0.0f, 0.0f, 0.0f),
	knockback_decay_(0.9f),
	player_start_timer_(0.0f),
	magnet_frag_(true)
{
	world_ = world;
	velocity_ = { player_speed_,player_speed_,0.0f };
	origin_position_ = position;
	transform_.position(position);
	actor_state_ = AC_Default;
	name_ = name;
	tag_ = tag;
	hp_ = player_max_hp_;
	scale_ = everything_scale_ * 0.2f;
	initialized_game_ = false;
	move_flag_ = false;
	online_state_ = 0;
	is_gauge_dec_ = 1.0f;

	//吸い込み制限関連
	magnet_gauge_ = MaxMagnetGauge;
	recovery_magnet_ = RecoveryMagnet;
	decrease_magnet_ = DecreaseMagnet;
	is_add_magnet_gauge_ = false;

	collider_ = BoundingSphere(13.0f * everything_scale_, GSvector3{ 0.0f,0.0f,0.0f });

	key_type_ = (name_ == player_1_name_) ? 0 : 1;

	ini_player_setting();

}

//コンストラクタ
Player::Player(IWorld* world, const GSvector3& position, const std::string& name, const std::string& tag, const int online_state, const int key_type, const float is_gauge_dec) :
	current_speed_(0.0f),
	acceleration_(0.08f),
	deceleration_(0.1f),
	recovery_timer_(0.0f),
	spown_alpha_(0.0f),
	magnet_effect_timer_(20.0f),
	knockback_velocity_(0.0f, 0.0f, 0.0f),
	knockback_decay_(0.9f),
	player_start_timer_(0.0f),
	magnet_frag_(true)
{
	world_ = world;
	velocity_ = { player_speed_,player_speed_,0.0f };
	origin_position_ = position;
	transform_.position(position);
	actor_state_ = AC_Default;
	name_ = name;
	tag_ = tag;
	hp_ = player_max_hp_;
	scale_ = everything_scale_ * 0.2f;
	initialized_game_ = false;
	move_flag_ = false;

	magnet_gauge_ = MaxMagnetGauge;
	recovery_magnet_ = RecoveryMagnet;
	decrease_magnet_ = DecreaseMagnet;
	is_add_magnet_gauge_ = false;
	is_gauge_dec_ = is_gauge_dec;

	collider_ = BoundingSphere(13.0f * everything_scale_, GSvector3{ 0.0f,0.0f,0.0f });

	online_state_ = online_state;
	key_type_ = key_type;
	ini_player_setting();

}

void Player::ini_player_setting() {

	//操作キー登録
	if (key_type_ == 0) {
		key_code_.push_back(0);
		key_code_.push_back(GKEY_W);
		key_code_.push_back(GKEY_S);
		key_code_.push_back(GKEY_A);
		key_code_.push_back(GKEY_D);
		key_code_.push_back(GKEY_R);
	}

	else if (key_type_ == 1) {
		key_code_.push_back(1);
		key_code_.push_back(GKEY_I);
		key_code_.push_back(GKEY_K);
		key_code_.push_back(GKEY_J);
		key_code_.push_back(GKEY_L);
		key_code_.push_back(GKEY_Y);
	}

	float core_x = 0.0f;

	//コアの位置
	//Player1だったら
	if (name_ == player_1_name_) {

		core_x = field_min_.x + core_x_plus_;
	}
	else if (name_ == player_2_name_ || name_ == cpu_name_) {
		core_x = field_max_.x - core_x_plus_;
	}

	if (is_gauge_dec_ > 0.0f)world_->add_actor(new Core{ world_,"CoreTag",name_,GSvector3{core_x,0.0f,0.0f} ,false ,online_state_});
	initialized_game_ = false;

	//コントローラー系一括初期化
	for (int i = 0; i < 5; i++) {

		ini_button_manager_.push_back(false);
		button_manager_.push_back(false);
		before_button_manager_.push_back(false);

		ini_button_manager_client_.push_back(0);
		button_manager_client_.push_back(0);

		button_cool_time_manager_.push_back(0.0f);

	}

}

//更新
void Player::update(float delta_time)
{

	if (world_->in_game_time(world_timer_id_) <= 0.0f)return;

	//チュートリアルなら何もしない
	if (name_ == player_2_name_ && is_gauge_dec_ <= 0.0f)return;

	if (!initialized_game_ && world_->next_scene() == 0)
	{
		reset();
		initialized_game_ = true;
	}

	//Break状態だったら当たり判定を無効にする
	if (actor_state_ == AC_Break)
	{
		enable_collider_ = false;
	}
	else
	{
		enable_collider_ = true;
	}

	if (world_->next_scene() == 4) initialized_game_ = false;

	//出現演出
	if (!move_flag_)
	{
		enable_collider_ = false;
		player_start_timer_ += delta_time;

		spown_alpha_ = CLAMP(player_start_timer_ / 60.0f, 0.0f, 1.0f);

		//だんだんシールドが回復していく
		if (player_start_timer_ > 60.0f)
		{
			if (name_ == player_1_name_) image_handle_1 = Image_Player_Default;
			else						 image_handle_2 = Image_Player2_Default;
			move_flag_ = true;
			enable_collider_ = true;
		}
		else if (player_start_timer_ > 40.0f)
		{
			if (name_ == player_1_name_) image_handle_1 = Image_Player_Damage1;
			else						 image_handle_2 = Image_Player2_Damage1;
		}
		else if (player_start_timer_ > 20.0f)
		{
			if (name_ == player_1_name_) image_handle_1 = Image_Player_Damage2;
			else						 image_handle_2 = Image_Player2_Damage2;
		}
		else if (player_start_timer_ > 0.0f)
		{
			if (name_ == player_1_name_) image_handle_1 = Image_Player_Break;
			else						 image_handle_2 = Image_Player2_Break;
		}
	}
	else
	{
		spown_alpha_ = 1.0f;
	}

	//Player1だったら
	if (name_ == player_1_name_)
	{
		spown_color_ = { 1.0f,0.2f,0.2f,1.0f };
	}
	else
	{
		spown_color_ = { 0.1f,0.1f,1.0f,1.0f };
	}

	//出現エフェクト
	if (spown_flag_ == 0)
	{
		effect_on(Effect_Paddle_spown, GSvector3(0.0f, -25.0f, 0.0f),
			GSvector3(0.0f, 0.0f, 0.0f), GSvector3(20.0f, 20.0f, 20.0f));
		gsSetEffectColor(effect_handle_, &spown_color_);
		spown_flag_++;
	}

	if (world_->in_game_time(actor_timer_id_) <= 0.0f || move_flag_ == false)return;

	if (!enable_) {
		enable_time_ += delta_time;

		if (enable_time_ >= enable_time_max_) {

			enable_time_ = 0.0f;
			enable_ = true;
		}
	}

	update_state(delta_time);


	//プレイヤー移動処理
	controller(key_code_, delta_time);

	//マグネットゲージ回復処理
	magunet_gauge_system();

	//ライフが0ならBraek状態にする
	if (hp_ <= 0)
	{
		if (actor_state_ != AC_Break)gsPlaySE(SE_Break_ex);
		actor_state_ = AC_Break;
	}

	float hp_ratio_ = hp_ / player_max_hp_;

	int hp_stage_ = 3;

	if (hp_ratio_ < 0.3f)      hp_stage_ = 0; // Break
	else if (hp_ratio_ < 0.5f) hp_stage_ = 1; // Damage2
	else if (hp_ratio_ < 0.7f) hp_stage_ = 2; // Damage1

	if (hp_stage_ != prev_hp_stage_)
	{
		switch (hp_stage_)
		{
		case 2: gsPlaySE(SE_Damage_01); break;
		case 1: gsPlaySE(SE_Damage_02); break;
		case 0: gsPlaySE(SE_Damage_02); break;
		default: break;
		}
		prev_hp_stage_ = hp_stage_; // 更新
	}

	//j引き寄せゲージ溢れないように
	magnet_gauge_ = CLAMP(magnet_gauge_, 0.0f, MaxMagnetGauge);

}

//描画
void Player::draw()const
{
	if (actor_state_ == AC_Break || player_start_timer_ <= 0.0f) return;

	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);

	GSvector3 player_pos_ = transform_.position();
	static const GSrect rect_{ -image_size_.x / 2.0f,image_size_.y / 2.0f , image_size_.y / 2.0f, -image_size_.x / 2.0f };
	const GSvector2 scale = { scale_,scale_ };

	//体力状態によってアルファ値が変化
	//float hp_ratio_ = hp_ / max_hp_;
	//float alpha = CLAMP(hp_ratio_, 0.2f, 1.0f); //0.2(薄い)～1.0(濃い)(完全に透明にならないよう0.2で止める)

	//float alpha;

	/*if (actor_state_ == AC_Break)
	{
		alpha = 0.5f;
	}
	else
	{
		alpha = 1.0f;
	}*/

	//alpha *= spown_alpha_;

	GScolor color = { 1.0f,1.0f,1.0f,spown_alpha_ };


	//体力状態によってテクスチャを変える
	if (prev_hp_stage_ == 0)
	{
		if (name_ == player_1_name_) image_handle_1 = Image_Player_Break;
		else					  image_handle_2 = Image_Player2_Break;
	}
	else if (prev_hp_stage_ == 1)
	{
		if (name_ == player_1_name_) image_handle_1 = Image_Player_Damage2;
		else						 image_handle_2 = Image_Player2_Damage2;
	}
	else if (prev_hp_stage_ == 2)
	{
		if (name_ == player_1_name_) image_handle_1 = Image_Player_Damage1;
		else						 image_handle_2 = Image_Player2_Damage1;
	}

	if (name_ == player_1_name_)
	{
		gsDrawSprite3D(image_handle_1, &player_pos_, &rect_, NULL, &color, &scale, 0.0f);
	}
	else
	{
		gsDrawSprite3D(image_handle_2, &player_pos_, &rect_, NULL, &color, &scale, 0.0f);
	}

	glPopAttrib();

}

//衝突リアクション
void Player::react(Actor& other)
{
	if (actor_state_ == AC_Break) return;

	if (other.name() == "Paddle")
	{

		world_->add_actor(new RecoverEffect{ world_,name_,transform_.position(),MagnetRecoverEffectTime,Effect_Magnet_Gauge_Recover,MagnetRecoverEffectAddPosition,MagnetRecoverEffectAngle,MagnetRecoverEffectScale });
		hp_ -= other.damage();
		//喰らったダメージと同じ割合だけ引き寄せゲージをヒール
		magnet_gauge_ += Magnet_gauge_rate * MaxMagnetGauge * (other.damage() / paddle_damage_max_);

		GSvector3 paddle_vel_ = other.save_velocity();

		if (actor_state_ == AC_Magnet || (actor_state_ == AC_Round && other.owner_name() == name_))
		{
			// 2点間の距離
			GSvector3 diff = transform_.position() - other.transform().position();
			GSvector3 dir = diff.getNormalized();
			float power = paddle_vel_.length();

			knockback_velocity_ = dir * power * 0.9f;
			actor_state_ = AC_Knockback;

		}
	}

	if (other.name() == "Core" + name_)
	{
		BoundingSphere core_col_ = other.collider();	 //コアの当たり判定(円)

		//2点間の距離
		GSvector3 diff = transform_.position() - other.transform().position();
		float dist = diff.length();

		//めり込み量=(半径の和-距離)
		float overlap = (collider().radius + core_col_.radius) - dist;

		if (dist < (collider().radius + core_col_.radius))
		{
			GSvector3 v = diff.getNormalized() * overlap;
			transform_.translate(v, GStransform::Space::World);
		}
	}
}

void Player::draw_gui()const
{
	if (is_gauge_dec_ <= 0.0f)return;

	float gauge_rate = magnet_gauge_ / MaxMagnetGauge; // 0.0 ～ 1.0
	gauge_rate = CLAMP(gauge_rate, 0.0f, 1.0f);

	float recovery_rate = recovery_timer_ / RecoveryTimeMax;

	GSvector2 gauge_pos = (name_ == player_1_name_)
		? GSvector2{ (ScreenWidth / 2.0f) - pos_offset, 180.0f } : GSvector2{ (ScreenWidth / 2.0f) + pos_offset, 180.0f };
	GSvector2 gauge_size = { 960.0f, 256.0f };
	GSrect gauge_rect = { 0.0f,0.0f,gauge_size.x,gauge_size.y };

	GSrect magnet_rect = (actor_state_ == AC_Break) ? GSrect{ 0.0f,0.0f,-gauge_size.x * recovery_rate,gauge_size.y } : GSrect{ 0.0f,0.0f,gauge_size.x * gauge_rate,gauge_size.y };

	GSvector2 gauge_center = { gauge_size.x / 2.0f,gauge_size.y / 2.0f };
	GSvector2 gauge_scale = (name_ == player_1_name_)
		? GSvector2{ -0.7f,0.3f } : GSvector2{ 0.7f,0.3f };
	GScolor magnet_red_color = { 1.0f,1.0f,1.0f,0.8f };
	GScolor magnet_blue_color = { 1.0f,1.0f,1.0f,0.8f };

	if (name_ == player_1_name_)
	{
		gsDrawSprite2D(Image_UI_Magnet_Gauge, &gauge_pos, &gauge_rect, &gauge_center, &magnet_red_color, &gauge_scale, 0.0f);
		gsDrawSprite2D(Image_UI_Magnet_, &gauge_pos, &magnet_rect, &gauge_center, &magnet_red_color, &gauge_scale, 0.0f);
	}
	else
	{
		gsDrawSprite2D(Image_UI_Magnet_Gauge, &gauge_pos, &gauge_rect, &gauge_center, &magnet_blue_color, &gauge_scale, 0.0f);
		gsDrawSprite2D(Image_UI_Magnet_, &gauge_pos, &magnet_rect, &gauge_center, &magnet_red_color, &gauge_scale, 0.0f);
	}
}

void Player::update_state(float delta_time)
{
	Actor* paddle = world_->find_actor(paddle_name_);

	if (!paddle) return;

	switch (actor_state_)
	{
	case AC_Magnet:
		magnet_gauge_ -= decrease_magnet_ * delta_time * is_gauge_dec_;

		if (magnet_gauge_ < 0.0f)
		{
			magnet_gauge_ = 0.0f;
			magnet_frag_ = false;
			actor_state_ = AC_Default;
		}

		//ボタンが押されている場合はループでエフェクトを出し続ける
		magnet_effect_timer_ -= delta_time;
		if (magnet_effect_timer_ <= 0.0f)
		{
			GScolor color = { 1.0f,1.0f,1.0f,0.7f };
			world_->add_actor(new EffectTexturePlayer
				{ world_,transform_.position(),false,"",
				Image_Effect_Magnet_Suction,GSvector2{256.0f,256.0f},
				GSvector2{1024.0f,512.0f},2.0f,8,paddle->magunet_range() / 300.0f,color });

			magnet_effect_timer_ = 20.0f;
		}

		if (util_.TargetDistance(paddle->transform().position(), transform_.position()) <= paddle_round_distance_) {

			actor_state_ = AC_Round;

		}

		break;

	case AC_Round:

		break;

	case AC_Default:
		//if (magnet_gauge_ < MaxMagnetGauge)
		//{
		//	magnet_gauge_ += recovery_magnet_;
		if (magnet_gauge_ > MaxMagnetGauge * MagnetStanLine)
		{
			magnet_frag_ = true;
		}
		//}
		magnet_effect_timer_ = 0.0f;
		break;

	case AC_Shoot:
		magnet_effect_timer_ = 0.0f;
		break;

	case AC_Break:
		// 初回突入時のみリセット
		if (break_timer_ == 0.0f)
		{
			break_effect_count_ = 0;
		}

		break_timer_ += delta_time;

		//エフェクトを6フレームごとに5回出す
		if (break_effect_count_ < 5 && break_timer_ >= interval * (break_effect_count_ + 1) && break_timer_ < 30.0f)
		{
			break_effect_count_++;

			// ランダムオフセットを作成（プレイヤー付近に出す）
			GSvector3 random_offset(
				util_.RandomRange(-15.0f, 15.0f),
				util_.RandomRange(-15.0f, 5.0f),
				0.0f
			);

			// エフェクト再生
			effect_on(
				Effect_Player_break,              // エフェクトID
				random_offset,                    // 相対位置（transform_からのオフセット）
				GSvector3(0.0f, 0.0f, 0.0f),      // 回転なし
				GSvector3(20.0f, 20.0f, 20.0f)    // スケール
			);
		}
		//30フレーム経過で消滅
		if (break_timer_ > 30.0f)
		{
			enable_ = false;          // 描画停止
			enable_collider_ = false; // 当たり判定も無効
			recovery_timer_ -= delta_time; //復活タイマー

			//5秒たったら
			if (recovery_timer_ <= -RecoveryTimeMax)
			{

				recovery_timer_ = 0.0f;
				actor_state_ = AC_Default;
				break_timer_ = 0.0f;
				hp_ = player_max_hp_;
				transform_.position(origin_position_); //位置を初期位置に戻す　

				//出現エフェクト
				effect_on(Effect_Paddle_spown, GSvector3(0.0f, -25.0f, 0.0f),
					GSvector3(0.0f, 0.0f, 0.0f), GSvector3(20.0f, 20.0f, 20.0f));
				gsSetEffectColor(effect_handle_, &spown_color_); //エフェクトの色設定

				gsPlaySE(SE_Shieldrepair_ex);

				spown_flag_ = 0;
				move_flag_ = false;
				enable_ = false;
				player_start_timer_ = 0.0f;
				//magnet_gauge_ = MaxMagnetGauge;
				magnet_frag_ = true;

			}
		}
		break;

	case AC_Knockback:
		//ノックバックを反映
		transform_.translate(knockback_velocity_, GStransform::Space::World);

		//減速
		knockback_velocity_ *= knockback_decay_;

		//クランプ
		GSvector3 pos = transform_.position();



		//プレイヤーサイズ(colliderのradiusと同じ)
		float player_size_ = 13.0f;


		transform_.position(pos);

		//ノックバックが終わったら
		if (knockback_velocity_.length() < 0.01f)
		{
			actor_state_ = AC_Default;
			knockback_velocity_ = GSvector3{ 0,0,0 };
		}

		break;
	}
}

void Player::controller(vector<GSint>& key, float delta_time) {

	GSvector3 move = { 0.0f,0.0f,0.0f };

	button_manager_ = ini_button_manager_;

	Actor* paddle = world_->find_actor("Paddle");
	Actor* core = world_->find_actor("Core" + name_);
	Actor* cpu = world_->find_actor(cpu_controler_name_);

	BoundingSphere player_col_ = collider();        //プレイヤーの当たり判定(円)

	stic_ = { 0.0f,0.0f };
	gsXBoxPadGetLeftAxis(key.at(0), &stic_);

	button_push(key);

	GSvector3 pos = transform_.position();			//プレイヤーの位置

	if (actor_state_ == AC_Knockback) {}

	//Break状態レバガチャ処理
	else if (actor_state_ == AC_Break) {

		//長押しで行ける要修正
		for (int i = 0; i < 5; i++) {

			if (button_manager_.at(i) && button_cool_time_manager_.at(i) <= 0.0f) {

				button_cool_time_manager_.at(i) = RebaGatyaCoolTime;
				recovery_timer_ -= DecRecoveryTime;
				break;

			}

			else if (!button_manager_.at(i)) {

				button_cool_time_manager_.at(i) -= delta_time;

			}

		}

	}

	else {

		if (paddle)
		{
			BoundingSphere paddle_col_ = paddle->collider(); //パドルの当たり判定(円)

			//2点間の距離
			GSvector3 diff = transform_.position() - paddle->transform().position();
			float dist = diff.length();

			//パドルとプレイヤーが重なっていたら
			if (dist < (player_col_.radius + paddle_col_.radius))
			{
				//めり込み量=(半径の和-距離)
				float overlap = (player_col_.radius + paddle_col_.radius) - dist;

				if (dist > 0.0f)
				{
					GSvector3 pushDir = diff / dist;

					//プレイヤーを押し戻す
					pos += pushDir * overlap;
				}
			}
		}

		//CPU
		if (cpu && name_ == cpu_name_)
		{
			GSvector3 cpu_vel_ = cpu->velocity();

			//CLAMP(player_speed_の範囲内に制限)
			cpu_vel_.x = CLAMP(cpu_vel_.x, -player_speed_, player_speed_);
			cpu_vel_.y = CLAMP(cpu_vel_.y, -player_speed_, player_speed_);

			velocity_ = cpu_vel_;

			move = { 1.0f,1.0f,0.0f };

			pos += move * velocity_ * delta_time;
		}
		//Player
		else if (online_state_ != online_recive_side_) {

			if (button_manager_.at(0)) move.y += 1.0f; //上
			if (button_manager_.at(1))  move.y -= 1.0f; //下
			if (button_manager_.at(2)) move.x -= 1.0f; //右
			if (button_manager_.at(3))  move.x += 1.0f; //左

			//正規化(斜め移動対応)
			if (move.length() > 0.0f)
			{
				move.normalize();

				//加速
				current_speed_ += acceleration_ * delta_time;
				if (current_speed_ > velocity_.x)
				{
					current_speed_ = velocity_.x;
				}
				if (current_speed_ > velocity_.y)
				{
					current_speed_ = velocity_.y;
				}
			}
			else
			{
				//減速
				current_speed_ -= deceleration_ * delta_time;
				if (current_speed_ < 0.0f)
				{
					current_speed_ = 0.0f;
				}
			}

			pos += move * current_speed_ * delta_time;
		}

		//Shoot
		int prev_state_ = actor_state_;

		if (name_ == cpu_name_) {
			// CPUが引き寄せる
			if (cpu && cpu->shoot_flag()&&magnet_frag_ && magnet_gauge_ > 0.0f) {

				if (actor_state_ != AC_Round) {

					actor_state_ = AC_Magnet;
				}

			}
			else {
				if (actor_state_ == AC_Magnet || actor_state_ == AC_Round) {
					actor_state_ = AC_Shoot;
					//gsPlaySE(SE_Shoot_ex);
				}
				else {
					actor_state_ = AC_Default;
				}
			}
		}
		else {
			// プレイヤー入力
			if (button_manager_.at(4)) {
				if (magnet_frag_ && magnet_gauge_ > 0.0f)
				{
					if (actor_state_ != AC_Round) {

						actor_state_ = AC_Magnet;

					}
				}
			}
			else {
				if (actor_state_ == AC_Magnet || actor_state_ == AC_Round) {
					actor_state_ = AC_Shoot;
					//gsPlaySE(SE_Shoot_ex);
				}
				else {
					actor_state_ = AC_Default;
				}
			}
		}

		//引き寄せの時一度だけ音を鳴らす
		if (prev_state_ != AC_Magnet && actor_state_ == AC_Magnet)
		{
			gsPlaySE(SE_Suction_ex);
		}

		int playerside = (name_ == player_1_name_) ? 0 : 1;
		GSvector3 left_top = world_->field_left_top(playerside);
		GSvector3 right_bottom = world_->field_right_bottom(playerside);

		//プレイヤーサイズ(colliderのradiusと同じ)
		float player_size_ = 13.0f;
		//プレイヤー１だったら
		if (name_ == player_1_name_)
		{
			//中央より右に行かない
			pos.x = CLAMP(pos.x, left_top.x, -player_size_ + right_bottom.x/*world_->center_line(transform_.position().y)*/);
			pos.y = CLAMP(pos.y, right_bottom.y, left_top.y);
		}
		//プレイヤー２またはCPUだったら
		else if (name_ == player_2_name_ || name_ == cpu_name_)
		{
			//中央より左に行かない
			pos.x = CLAMP(pos.x, player_size_ + left_top.x/*world_->center_line(transform_.position().y)*/, right_bottom.x);
			pos.y = CLAMP(pos.y, right_bottom.y, left_top.y);
		}

		transform_.position(pos);

	}

	//オンライン用
	creat_data();

}

//押されたボタンに応じてtruefalse返す
void Player::button_push(vector<GSint>& key) {

	if (online_state_ == 2) {

		online_controller();

	}

	else {

		//操作一括
		if (gsGetKeyState(key.at(1)) || gsXBoxPadButtonState(key.at(0), GS_XBOX_PAD_UP) || stic_.y > 0.0f)	button_manager_.at(0) = true; //上
		if (gsGetKeyState(key.at(2)) || gsXBoxPadButtonState(key.at(0), GS_XBOX_PAD_DOWN) || stic_.y < 0.0f)  button_manager_.at(1) = true; //下
		if (gsGetKeyState(key.at(3)) || gsXBoxPadButtonState(key.at(0), GS_XBOX_PAD_LEFT) || stic_.x < 0.0f) button_manager_.at(2) = true; //左
		if (gsGetKeyState(key.at(4)) || gsXBoxPadButtonState(key.at(0), GS_XBOX_PAD_RIGHT) || stic_.x > 0.0f)  button_manager_.at(3) = true; //右
		if (gsGetKeyState(key.at(5)) || gsXBoxPadButtonState(key.at(0), GS_XBOX_PAD_A))button_manager_.at(4) = true;//シュート

	}

}

//送信データつくるやつ
void Player::creat_data() {

	if (online_state_ == online_send_side_) {

		packet_data_.clear();

		char s[512];
		snprintf(s, 512, ",%f,%f,%f,%f", transform_.position().x, transform_.position().y, hp_,magnet_gauge_);

		for (int i = 0; i < 5; i++) {

			packet_data_ += creat_packet_data(button_manager_.at(i));

		}

		packet_data_ += s;

		world_->send_data_online(packet_data_, send_data_id_player_);
		world_->change_is_send_data(send_data_id_player_, true);

	}

}

//オンラインの対戦相手動かすやつ
void Player::online_controller() {

	//データ受け取ったなら
	if (!world_->is_player_got_data(send_data_id_player_)) {

		//中身があったら
		if (world_->send_data_to_player_packet(send_data_id_player_).size() > 0) {

			string pos_name = "プレイヤー";
			cut_data(pos_name,send_data_id_player_);

			for (int i = 0; i < 5; i++) {

				button_manager_.at(i) = stoi(recive_packet_data_.at(i));

			}

			GSvector3 pos{ stof(recive_packet_data_.at(GD_PosX)),stof(recive_packet_data_.at(GD_PosY)),0.0f };
			transform_.position(pos);
			hp_ = stof(recive_packet_data_.at(GD_Hp));
			magnet_gauge_ = stof(recive_packet_data_.at(GD_Magunet_Gauge));

		}

		//受け取り完了
		world_->change_is_player_got_data(send_data_id_player_, true);

	}

}

//相手プレイヤーに送るやつ作る
string Player::creat_packet_data(bool data) {

	int data_change = (data) ? 1 : 0;
	char s[32];
	snprintf(s, 32, ",%d", data_change);

	return s;

}

//リセット
void Player::reset() {
	velocity_ = { player_speed_,player_speed_,0.0f };
	hp_ = player_max_hp_;
	transform_.position(origin_position_);		//場所初期化
	actor_state_ = AC_Default;
	image_handle_1 = Image_Player_Default;
	player_start_timer_ = 0.0f;
	spown_flag_ = 0;
	move_flag_ = false;
	enable_ = false;
	spown_alpha_ = 0.0f;
	magnet_gauge_ = MaxMagnetGauge;
	magnet_frag_ = true;
	is_add_magnet_gauge_ = false;

	if (name_ == player_1_name_) {
		//相手がplayer2かcpuか
		if (world_->find_actor(cpu_controler_name_))
		{
			enemy_name_ = cpu_name_;
		}
		else
		{
			enemy_name_ = player_2_name_;
		}
	}

	else if (name_ == player_2_name_ || name_ == cpu_name_) {
		enemy_name_ = player_1_name_;
	}
}


void Player::magunet_gauge_system() {

	Actor* enemy = world_->find_actor(enemy_name_);
	if (enemy == nullptr)return;

	if (enemy->actor_state() == AC_Break) {

		//ヒールを1回だけにする
		if (!is_add_magnet_gauge_) {

			is_add_magnet_gauge_ = true;
			magnet_gauge_ += recovery_magnet_;
			world_->add_actor(new RecoverEffect{ world_,name_,transform_.position(),MagnetRecoverEffectTime,Effect_Magnet_Gauge_Recover,MagnetRecoverEffectAddPosition,MagnetRecoverEffectAngle,MagnetRecoverEffectScale });
			gsPlaySE(SE_Magnet_Recovery);
		}

	}

	else {

		is_add_magnet_gauge_ = false;

	}

}