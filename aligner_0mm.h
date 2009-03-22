/*
 * aligner_0mm.h
 */

#ifndef ALIGNER_0MM_H_
#define ALIGNER_0MM_H_

#include <utility>
#include <vector>
#include "aligner.h"
#include "hit.h"
#include "row_chaser.h"
#include "range_chaser.h"

/**
 * Concrete factory class for constructing unpaired exact aligners.
 */
class UnpairedExactAlignerV1Factory : public AlignerFactory {

	typedef RangeSourceDriver<EbwtRangeSource> TRangeSrcDr;
	typedef std::vector<TRangeSrcDr*> TRangeSrcDrPtrVec;
	typedef CostAwareRangeSourceDriver<EbwtRangeSource> TCostAwareRangeSrcDr;

public:
	UnpairedExactAlignerV1Factory(
			Ebwt<String<Dna> >& ebwtFw,
			Ebwt<String<Dna> >* ebwtBw,
			bool doFw,
			bool doRc,
			HitSink& sink,
			const HitSinkPerThreadFactory& sinkPtFactory,
			RangeCache* cacheFw,
			RangeCache* cacheBw,
			uint32_t cacheLimit,
			vector<String<Dna5> >& os,
			bool strandFix,
			bool rangeMode,
			bool verbose,
			uint32_t seed) :
			ebwtFw_(ebwtFw),
			ebwtBw_(ebwtBw),
			doFw_(doFw), doRc_(doRc),
			sink_(sink),
			sinkPtFactory_(sinkPtFactory),
			cacheFw_(cacheFw),
			cacheBw_(cacheBw),
			cacheLimit_(cacheLimit),
			os_(os),
			maqPenalty_(false),
			strandFix_(strandFix),
			rangeMode_(rangeMode),
			verbose_(verbose),
			seed_(seed)
	{
		assert(ebwtFw.isInMemory());
	}

	/**
	 * Create a new UnpairedExactAlignerV1s.
	 */
	virtual Aligner* create() const {
		HitSinkPerThread* sinkPt = sinkPtFactory_.create();
		EbwtSearchParams<String<Dna> >* params =
			new EbwtSearchParams<String<Dna> >(*sinkPt, os_, true, true, true, rangeMode_);

		EbwtRangeSource *rFw = new EbwtRangeSource(
			&ebwtFw_, true,  0xffffffff, true, false, seed_, false, false);
		EbwtRangeSource *rRc = new EbwtRangeSource(
			&ebwtFw_, false, 0xffffffff, true, false, seed_, false, false);

		EbwtRangeSourceDriver * driverFw = new EbwtRangeSourceDriver(
			*params, rFw, true, false, maqPenalty_, sink_, sinkPt,
			0,          // seedLen
			true,       // nudgeLeft (not applicable)
			PIN_TO_LEN, // whole alignment is unrevisitable
			PIN_TO_LEN, // "
			PIN_TO_LEN, // "
			PIN_TO_LEN, // "
			os_, verbose_, seed_, true);
		EbwtRangeSourceDriver * driverRc = new EbwtRangeSourceDriver(
			*params, rRc, false, false, maqPenalty_, sink_, sinkPt,
			0,          // seedLen
			true,       // nudgeLeft (not applicable)
			PIN_TO_LEN, // whole alignment is unrevisitable
			PIN_TO_LEN, // "
			PIN_TO_LEN, // "
			PIN_TO_LEN, // "
			os_, verbose_, seed_, true);
		TRangeSrcDrPtrVec drVec;
		if(doFw_) drVec.push_back(driverFw);
		if(doRc_) drVec.push_back(driverRc);
		TCostAwareRangeSrcDr* dr = new TCostAwareRangeSrcDr(seed_, strandFix_, drVec);

		// Set up a RangeChaser
		RangeChaser<String<Dna> > *rchase =
			new RangeChaser<String<Dna> >(seed_, cacheLimit_, cacheFw_, cacheBw_);

		return new UnpairedAlignerV2<EbwtRangeSource>(
			params, dr, rchase,
			sink_, sinkPtFactory_, sinkPt, os_, rangeMode_, verbose_,
			seed_);
	}

private:
	Ebwt<String<Dna> >& ebwtFw_;
	Ebwt<String<Dna> >* ebwtBw_;
	bool doFw_;
	bool doRc_;
	HitSink& sink_;
	const HitSinkPerThreadFactory& sinkPtFactory_;
	RangeCache *cacheFw_;
	RangeCache *cacheBw_;
	const uint32_t cacheLimit_;
	vector<String<Dna5> >& os_;
	bool maqPenalty_;
	bool strandFix_;
	bool rangeMode_;
	bool verbose_;
	uint32_t seed_;
};

/**
 * Concrete factory class for constructing unpaired exact aligners.
 */
class PairedExactAlignerV1Factory : public AlignerFactory {
public:
	PairedExactAlignerV1Factory(
			Ebwt<String<Dna> >& ebwtFw,
			Ebwt<String<Dna> >* ebwtBw,
			HitSink& sink,
			const HitSinkPerThreadFactory& sinkPtFactory,
			bool mate1fw,
			bool mate2fw,
			uint32_t peInner,
			uint32_t peOuter,
			bool dontReconcile,
			uint32_t symCeil,
			uint32_t mixedThresh,
			uint32_t mixedAttemptLim,
			RangeCache* cacheFw,
			RangeCache* cacheBw,
			uint32_t cacheLimit,
			BitPairReference* refs,
			vector<String<Dna5> >& os,
			bool strandFix,
			bool rangeMode,
			bool verbose,
			uint32_t seed) :
			ebwtFw_(ebwtFw),
			sink_(sink),
			sinkPtFactory_(sinkPtFactory),
			mate1fw_(mate1fw),
			mate2fw_(mate2fw),
			peInner_(peInner),
			peOuter_(peOuter),
			dontReconcile_(dontReconcile),
			symCeil_(symCeil),
			mixedThresh_(mixedThresh),
			mixedAttemptLim_(mixedAttemptLim),
			cacheFw_(cacheFw),
			cacheBw_(cacheBw),
			cacheLimit_(cacheLimit),
			refs_(refs), os_(os),
			maqPenalty_(false),
			strandFix_(strandFix),
			rangeMode_(rangeMode),
			verbose_(verbose),
			seed_(seed)
	{
		assert(ebwtFw.isInMemory());
	}

	/**
	 * Create a new UnpairedExactAlignerV1s.
	 */
	virtual Aligner* create() const {
		HitSinkPerThread* sinkPt = sinkPtFactory_.createMult(2);
		EbwtSearchParams<String<Dna> >* params =
			new EbwtSearchParams<String<Dna> >(*sinkPt, os_, true, true, true, rangeMode_);

		EbwtRangeSource *r1Fw = new EbwtRangeSource(
			&ebwtFw_, true,  0xffffffff, true, false, seed_, false, false);
		EbwtRangeSource *r1Rc = new EbwtRangeSource(
			&ebwtFw_, false, 0xffffffff, true, false, seed_, false, false);

		EbwtRangeSourceDriver * driver1Fw = new EbwtRangeSourceDriver(
			*params, r1Fw, true, false, maqPenalty_, sink_, sinkPt,
			0,          // seedLen
			true,       // nudgeLeft (not applicable)
			PIN_TO_LEN, // whole alignment is unrevisitable
			PIN_TO_LEN, // "
			PIN_TO_LEN, // "
			PIN_TO_LEN, // "
			os_, verbose_, seed_, true);
		EbwtRangeSourceDriver * driver1Rc = new EbwtRangeSourceDriver(
			*params, r1Rc, false, false, maqPenalty_, sink_, sinkPt,
			0,          // seedLen
			true,       // nudgeLeft (not applicable)
			PIN_TO_LEN, // whole alignment is unrevisitable
			PIN_TO_LEN, // "
			PIN_TO_LEN, // "
			PIN_TO_LEN, // "
			os_, verbose_, seed_, true);

		EbwtRangeSource *r2Fw = new EbwtRangeSource(
			&ebwtFw_, true,  0xffffffff, true, false, seed_, false, false);
		EbwtRangeSource *r2Rc = new EbwtRangeSource(
			&ebwtFw_, false, 0xffffffff, true, false, seed_, false, false);

		EbwtRangeSourceDriver * driver2Fw = new EbwtRangeSourceDriver(
			*params, r2Fw, true, false, maqPenalty_, sink_, sinkPt,
			0,          // seedLen
			true,       // nudgeLeft (not applicable)
			PIN_TO_LEN, // whole alignment is unrevisitable
			PIN_TO_LEN, // "
			PIN_TO_LEN, // "
			PIN_TO_LEN, // "
			os_, verbose_, seed_, false);
		EbwtRangeSourceDriver * driver2Rc = new EbwtRangeSourceDriver(
			*params, r2Rc, false, false, maqPenalty_, sink_, sinkPt,
			0,          // seedLen
			true,       // nudgeLeft (not applicable)
			PIN_TO_LEN, // whole alignment is unrevisitable
			PIN_TO_LEN, // "
			PIN_TO_LEN, // "
			PIN_TO_LEN, // "
			os_, verbose_, seed_, false);

		RefAligner<String<Dna5> >* refAligner = new ExactRefAligner<String<Dna5> >(0);

		// Set up a RangeChaser
		RangeChaser<String<Dna> > *rchase =
			new RangeChaser<String<Dna> >(seed_, cacheLimit_, cacheFw_, cacheBw_);

		return new PairedBWAlignerV1<EbwtRangeSource>(
			params,
			driver1Fw, driver1Rc, driver2Fw, driver2Rc, refAligner,
			rchase, sink_, sinkPtFactory_, sinkPt, mate1fw_, mate2fw_,
			peInner_, peOuter_, dontReconcile_, symCeil_, mixedThresh_,
			mixedAttemptLim_, refs_, rangeMode_, verbose_, seed_);
	}

private:
	Ebwt<String<Dna> >& ebwtFw_;
	Ebwt<String<Dna> >* ebwtBw_;
	HitSink& sink_;
	const HitSinkPerThreadFactory& sinkPtFactory_;
	const bool mate1fw_;
	const bool mate2fw_;
	const uint32_t peInner_;
	const uint32_t peOuter_;
	const bool dontReconcile_;
	const uint32_t symCeil_;
	const uint32_t mixedThresh_;
	const uint32_t mixedAttemptLim_;
	RangeCache *cacheFw_;
	RangeCache *cacheBw_;
	const uint32_t cacheLimit_;
	BitPairReference* refs_;
	vector<String<Dna5> >& os_;
	const bool maqPenalty_;
	const bool strandFix_;
	const bool rangeMode_;
	const bool verbose_;
	const uint32_t seed_;
};

#endif /* ALIGNER_0MM_H_ */
