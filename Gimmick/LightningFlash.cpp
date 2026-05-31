#include"Actor/Gimmick/LightningFlash.h"
#include"Util/UtilNumber.h"
#include"Actor/System/GimmickDetect.h"

static const float LF_Size{ 11.0f };
static const GSvector2 LFTextureSize{ 256.0f,256.0f };
static const float LFScale{ 0.08f };
static const float flash_max_timer_{ 120.0f };
static const float wait_max_timer_{ 68.0f };
const float max_hp_{ 1.0f };
float flash_side_{ 0.0f };

LightningFlash::LightningFlash(IWorld* world, const GSvector3& position, const int delete_num)
{
	world_ = world;
	name_ = "LightningFlash";
	tag_ = gimmick_tag_;
	collider_ = BoundingSphere{ LF_Size * everything_scale_,GSvector3{0.0f,0.0f,0.0f} };
	transform_.position(position);
	origin_position_ = transform_.position();
	lf_state_ = LF_Default;
	hp_ = max_hp_;
	actor_handle_ = world_->add_actor_handle();
	image_size_ = LFTextureSize;
	scale_ = everything_scale_ * LFScale;
	flash_timer_ = flash_max_timer_;
	player1_side_ = false;
	player2_side_ = false;
	delete_positon_id_ = delete_num;
}

void LightningFlash::update(float delta_time)
{

	if ((world_->next_scene() == 4 && world_->stage_make_type() == 1) || world_->gimmick_delete_frag()) {

		die();

	}

	switch (lf_state_)
	{
	case LF_Default:

		break;

	case LF_Wait:
		if (flash_timer_ <= 0)
		{
			lf_state_ = LF_Flash;
			flash_timer_ = flash_max_timer_;
		}

		break;

	case LF_Flash:
		if (flash_timer_ <= 0)
		{
			if (delete_positon_id_ != -1)world_->add_actor(new GimmickDetect{ world_,delete_positon_id_ });
			die();
		}

		break;
	}

	flash_timer_ -= delta_time;

}

void LightningFlash::draw()const
{
	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);

	GSvector3 lf_pos_ = origin_position_;
	static const GSrect lf_rect_{ -image_size_.x / 2.0f,image_size_.y / 2.0f,image_size_.y / 2.0f,-image_size_.x / 2.0f };
	const GSvector2 lf_scale = { scale_,scale_ };

	if (lf_state_ == LF_Default)
	{
		gsDrawSprite3D(Image_Gimmick_LightningFlash, &lf_pos_, &lf_rect_, NULL, NULL, &lf_scale, 0.0f);
	}

	glPopAttrib();
	//collider().draw();
}

void LightningFlash::draw_transparent()const
{
	if (lf_state_ == LF_Flash)
	{
		glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_LIGHTING);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glEnable(GL_BLEND);

		float alpha = flash_timer_ / flash_max_timer_; // èôÅXÇ…îñÇ≠Ç∑ÇÈ
		float screen_half = ScreenWidth / 2.0f;

		if (player1_side_ == true)
		{
			flash_side_ = screen_half - (screen_half / 2.0f);
		}
		else if (player2_side_ == true)
		{
			flash_side_ = screen_half + (screen_half / 2.0f);
		}

		GSvector2 flash_image_size_ = { 960.0f,1080.0f };

		GSvector2 flash_pos_ = { flash_side_ ,ScreenHeight / 2.0f };
		static const GSrect flash_rect_{ 0.0f,0.0f,flash_image_size_.x,flash_image_size_.y };
		GSvector2 flash_center_ = { flash_image_size_.x / 2.0f,flash_image_size_.y / 2.0f };
		GScolor flash_color_ = { 1.0f, 1.0f, 1.0f, alpha };
		const GSvector2 flash_scale_ = { 1.0f,1.0f };

		gsDrawSprite2D(Image_Gimmick_Flash, &flash_pos_, &flash_rect_, &flash_center_, &flash_color_, &flash_scale_, 0.0f);
		glPopAttrib();
	}
}

void LightningFlash::react(Actor& other)
{
	if (other.name() == gimmick_detect_name_)return;
	
	if (lf_state_ != LF_Default) return;

	if (other.name() == "Paddle")
	{

		hp_ -= other.damage();
		std::string owner = other.owner_name();
		enable_collider_ = false;

		if (owner == player_1_name_)
		{
			player2_side_ = true;
		}
		else
		{
			player1_side_ = true;
		}

		//ÉâÉCÉtÇ™0Ç…Ç»Ç¡ÇΩÇÁ
		if (hp_ <= 0)
		{
			gsPlaySE(SE_LightnigFlash);
			lf_state_ = LF_Wait;
			flash_timer_ = wait_max_timer_;
		}
	}
}