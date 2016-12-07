#ifndef C_FRONTEND_H
#define C_FRONTEND_H

#include <vector>

#include "backend/IterativeRegionMerging.hpp"
#include "backend/MergeFunctions.hpp"
#include "backend/Operators.hpp"
#include "backend/types.hpp"
#include "evaluate.hpp"

typedef uint64_t                                                 SegID;
typedef uint32_t                                                 GtID;
typedef float                                                    AffValue;
typedef float                                                    ScoreValue;
typedef RegionGraph<SegID>                                       RegionGraphType;
typedef typename RegionGraphType::template EdgeMap<AffValue>     AffinitiesType;
typedef typename RegionGraphType::template NodeMap<std::size_t>  SizesType;
typedef IterativeRegionMerging<SegID, ScoreValue>                RegionMergingType;

//typedef Multiply<OneMinus<MaxAffinity<AffinitiesType>>, MinSize<SizesType>> ScoringFunctionType;
//typedef Multiply<OneMinus<MedianAffinity<AffinitiesType>>, MinSize<SizesType>> ScoringFunctionType;
typedef Divide<MinSize<SizesType>, Square<MaxAffinity<AffinitiesType>>> ScoringFunctionType;


struct Metrics {

	double voi_split;
	double voi_merge;
	double rand_split;
	double rand_merge;
};

struct WaterzState {

	int     context;
	Metrics metrics;
};

class WaterzContext {

public:

	static WaterzContext* createNew() {

		WaterzContext* context = new WaterzContext();
		context->id = _nextId;
		_nextId++;
		_contexts.insert(std::make_pair(context->id, context));

		return context;
	}

	static WaterzContext* get(int id) {

		if (!_contexts.count(id))
			return NULL;

		return _contexts.at(id);
	}

	static void free(int id) {

		WaterzContext* context = get(id);

		if (context) {

			_contexts.erase(id);
			delete context;
		}
	}

	int id;

	std::shared_ptr<RegionGraphType>     regionGraph;
	std::shared_ptr<AffinitiesType>      edgeAffinities;
	std::shared_ptr<SizesType>           regionSizes;

	std::shared_ptr<RegionMergingType>   regionMerging;
	std::shared_ptr<ScoringFunctionType> scoringFunction;
	volume_ref_ptr<SegID>                segmentation;
	volume_const_ref_ptr<GtID>           groundtruth;

private:

	WaterzContext() {}

	~WaterzContext() {}

	static std::map<int, WaterzContext*> _contexts;
	static int _nextId;
};

WaterzState initialize(
		size_t          width,
		size_t          height,
		size_t          depth,
		const AffValue* affinity_data,
		SegID*          segmentation_data,
		const GtID*     groundtruth_data = NULL,
		AffValue        affThresholdLow  = 0.0001,
		AffValue        affThresholdHigh = 0.9999);

void mergeUntil(
		WaterzState& state,
		float        threshold);

void free(WaterzState& state);

#endif
