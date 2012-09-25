/*
 * This class is being presented in the following web site, 
 * http://www.ogre3d.org/tikiwiki/AnimationBlender.
 */


#include <Ogre.h>

class AnimationBlender
{
public:
	enum BlendingTransition
	{
		BlendSwitch,         // stop source and start dest
		BlendWhileAnimating,   // cross fade, blend source animation out while blending destination animation in
		BlendThenAnimate      // blend source to first frame of dest, when done, start dest anim
	};

	Ogre::Real mTimeleft, mDuration;
	bool complete;

	AnimationBlender(Ogre::Entity *);
	~AnimationBlender() {}

	Ogre::Real getProgress() { return mTimeleft/ mDuration; }
	Ogre::AnimationState *getSource() { return mSource; }
	Ogre::AnimationState *getTarget() { return mTarget; }

	void init( const Ogre::String &animation, bool l=true );	
	void blend( const Ogre::String &animation, BlendingTransition transition, Ogre::Real duration, bool l=true );
	void addTime(Ogre::Real );
	
private:
	Ogre::Entity *mEntity;
	Ogre::AnimationState *mSource;
	Ogre::AnimationState *mTarget;
	
	BlendingTransition mTransition;
	
	bool loop;
};
