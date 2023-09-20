Scriptname BQRNG_SCPT_UpdateReward Extends REFERENCEALIAS

EVENT OnDeath(ACTOR akKiller)
	BQRNG.UpdateReward(Self.GetOwningQuest(), Self.GetID())
ENDEVENT
