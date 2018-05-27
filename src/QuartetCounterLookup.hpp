#pragma once

#include "genesis/genesis.hpp"
#include <vector>
#include <cassert>
#include <algorithm>
#include <memory>
#include "TreeInformation.hpp"
#include "quartet_lookup_table.hpp"
#include "QuartetScoreComputer.hpp"
#include "metaquartet_lookup_table.hpp"

#include "em_counting.hpp"

#include <unordered_map>
#include <cstdint>
#include <stxxl/vector>
#include <stxxl/parallel_sorter_synchron>
#include "easylogging++.h"
//#include <stxxl/sorter>

using namespace genesis;
using namespace tree;
using namespace utils;
using namespace std;

template class std::vector<size_t>;

#define CO(a,b,c,d) (a) * n_cube + (b) * n_square + (c) * n + (d)

template <typename T>
struct my_comparator
{
   bool operator () (const T& a, const T& b) const
   {
       return a < b;
   }

   T min_value() const
   {
       return std::numeric_limits<T>::min();
   }

   T max_value() const
   {
       return std::numeric_limits<T>::max();
   }
};

/**
 * Let n be the number of taxa in the reference tree.
 * Count occurrences of quartet topologies in the set of evaluation trees using a O(n^4) lookup table with O(1) lookup cost.
 */
template<typename CINT>
class QuartetCounterLookup {
private:
	using sorter_type = stxxl::parallel_sorter_synchron<uint64_t, my_comparator<uint64_t> >; 
public:
	QuartetCounterLookup(const Tree &refTree, const std::string &evalTreesPath, size_t m, bool verboseOutput, bool savemem, int num_threads, int internalMemory);
	~QuartetCounterLookup() = default;
	std::tuple<CINT, CINT, CINT> countQuartetOccurrences(size_t aIdx, size_t bIdx, size_t cIdx, size_t dIdx) const;
	std::unique_ptr<QuartetScoreComputer<CINT>> qsc;
private:
	void countQuartets(const std::string &evalTreesPath, size_t m,
			const std::unordered_map<std::string, size_t> &taxonToReferenceID,
			const uint64_t max_em_elements = 1llu << 34
	);
	void updateQuartets(const Tree &tree, size_t nodeIdx, const std::vector<int> &eulerTourLeaves,
			const std::vector<int> &linkToEulerLeafIndex, int t1, int t2);
	void updateQuartetsThreeLinks(size_t link1, size_t link2, size_t link3, const Tree &tree,
			const std::vector<int> &eulerTourLeaves, const std::vector<int> &linkToEulerLeafIndex, int t1, int t2);
	void updateQuartetsThreeClades(size_t startLeafIndexS1, size_t endLeafIndexS1, size_t startLeafIndexS2,
			size_t endLeafIndexS2, size_t startLeafIndexS3, size_t endLeafIndexS3,
			const std::vector<int> &eulerTourLeaves, int t1, int t2);
	std::pair<size_t, size_t> subtreeLeafIndices(size_t linkIdx, const Tree &tree,
			const std::vector<int> &linkToEulerLeafIndex);

	CINT lookupQuartetCount(size_t aIdx, size_t bIdx, size_t cIdx, size_t dIdx) const;

	std::vector<CINT> lookupTableFast; /**> larger O(n^4) lookup table storing the count of each quartet topology */
	QuartetLookupTable<CINT> lookupTable; /**> smaller O(n^4) lookup table storing the count of each quartet topology */

	size_t n; /**> number of taxa in the reference tree */
	size_t n_square; /**> n*n */
	size_t n_cube; /**> n*n*n */
	std::vector<size_t> refIdToLookupID;
	std::vector<size_t> lookupIdToRefId;
	bool savemem; /**> trade speed for less memory or not */
	std::vector<sorter_type> quartetSorter;
	em_counting<uint64_t, CINT> quartetCount;
	void reduceSorter();
	int nthread;
};

/**
 * Update the quartet topology counts for quartets  {a.b.c.d} where a,b \in S_1, c \in S_2, and d \in S_3.
 * @param startLeafIndexS1 the first index in eulerTourLeaves that corresponds to a leaf in subtree S_1
 * @param endLeafIndexS1 the last index in eulerTourLeaves that corresponds to a leaf in subtree S_1
 * @param startLeafIndexS2 the first index in eulerTourLeaves that corresponds to a leaf in subtree S_2
 * @param endLeafIndexS2 the last index in eulerTourLeaves that corresponds to a leaf in subtree S_2
 * @param startLeafIndexS3 the first index in eulerTourLeaves that corresponds to a leaf in subtree S_3
 * @param endLeafIndexS3 the last index in eulerTourLeaves that corresponds to a leaf in subtree S_3
 * @param eulerTourLeaves the leaves' IDs of the tree traversed in an euler tour order
 */
template<typename CINT>
void QuartetCounterLookup<CINT>::updateQuartetsThreeClades(size_t startLeafIndexS1, size_t endLeafIndexS1,
		size_t startLeafIndexS2, size_t endLeafIndexS2, size_t startLeafIndexS3, size_t endLeafIndexS3,
		const std::vector<int> &eulerTourLeaves, int t1, int t2) {
	size_t aLeafIndex = startLeafIndexS1;
	size_t bLeafIndex = startLeafIndexS2;
	size_t cLeafIndex = startLeafIndexS3;

	while (aLeafIndex != endLeafIndexS1) {
		size_t a = eulerTourLeaves[aLeafIndex];
		size_t a2LeafIndex = (aLeafIndex + 1) % eulerTourLeaves.size();
		while (a2LeafIndex != endLeafIndexS1) {
			size_t a2 = eulerTourLeaves[a2LeafIndex];
			while (bLeafIndex != endLeafIndexS2) {
				size_t b = eulerTourLeaves[bLeafIndex];
				while (cLeafIndex != endLeafIndexS3) {
					size_t c = eulerTourLeaves[cLeafIndex];

					if (savemem) {
						auto& tuple = lookupTable.get_tuple(a, a2, b, c);
						size_t tupleIdx = lookupTable.tuple_index(a, a2, b, c);
//#pragma omp atomic
						tuple[tupleIdx]++;
					} else {
//#pragma omp atomic

//size_t tmp = CO(a, a2, b, c);
//quartetSorter.push(tmp,t);						
uint64_t tmp = qsc->get_index(a,a2,b,c);
quartetSorter[t1].push(tmp,t2);	
//lookupTableFast[CO(a, a2, b, c)]++;
					}

					cLeafIndex = (cLeafIndex + 1) % eulerTourLeaves.size();
				}
				bLeafIndex = (bLeafIndex + 1) % eulerTourLeaves.size();
				cLeafIndex = startLeafIndexS3;
			}
			a2LeafIndex = (a2LeafIndex + 1) % eulerTourLeaves.size();
			bLeafIndex = startLeafIndexS2;
			cLeafIndex = startLeafIndexS3;
		}
		aLeafIndex = (aLeafIndex + 1) % eulerTourLeaves.size();
		bLeafIndex = startLeafIndexS2;
		cLeafIndex = startLeafIndexS3;
	}
}

/**
 * Return a pair <start, end> representing the leaf indices in the Euler tour within the subtree induced by the genesis TreeLink with ID linkIdx.
 * The leaf indices are between [start,end), this means they include the start index but not the end index.
 * @param linkIdx the ID of the TreeLink from genesis
 * @param tree the tree
 * @param linkToEulerLeafIndex Mapping of each link in the tree to indices in the euler tour;
 * 	needed for determining first and last index of leaves belonging to a subtree.
 */
template<typename CINT>
std::pair<size_t, size_t> QuartetCounterLookup<CINT>::subtreeLeafIndices(size_t linkIdx, const Tree &tree,
		const std::vector<int> &linkToEulerLeafIndex) {
	size_t outerLinkIdx = tree.link_at(linkIdx).outer().index();
	return {linkToEulerLeafIndex[linkIdx] % linkToEulerLeafIndex.size(), linkToEulerLeafIndex[outerLinkIdx] % linkToEulerLeafIndex.size()};
}

/**
 * Given the genesis links to the tree subtrees induced by an inner node, update the quartet topology counts of all quartets
 * {a,b,c,d} for which a and b are in the same subtree, c is in another subtree, and d is in the remaining subtree.
 * @param link1 link ID to the first subtree
 * @param link2 link ID to the second subtree
 * @param link3 link ID to the third subtree
 * @param tree the evaluation tree
 * @param eulerTourLeaves the leaves' IDs of the tree traversed in an euler tour order
 * @param linkToEulerLeafIndex Mapping of each link in the tree to indices in the euler tour;
 * 	needed for determining first and last index of leaves belonging to a subtree.
 */
template<typename CINT>
void QuartetCounterLookup<CINT>::updateQuartetsThreeLinks(size_t link1, size_t link2, size_t link3, const Tree &tree,
		const std::vector<int> &eulerTourLeaves, const std::vector<int> &linkToEulerLeafIndex, int t1, int t2) {
	std::pair<size_t, size_t> subtree1 = subtreeLeafIndices(link1, tree, linkToEulerLeafIndex);
	std::pair<size_t, size_t> subtree2 = subtreeLeafIndices(link2, tree, linkToEulerLeafIndex);
	std::pair<size_t, size_t> subtree3 = subtreeLeafIndices(link3, tree, linkToEulerLeafIndex);

	size_t startLeafIndexS1 = subtree1.first % eulerTourLeaves.size();
	size_t endLeafIndexS1 = subtree1.second % eulerTourLeaves.size();
	size_t startLeafIndexS2 = subtree2.first % eulerTourLeaves.size();
	size_t endLeafIndexS2 = subtree2.second % eulerTourLeaves.size();
	size_t startLeafIndexS3 = subtree3.first % eulerTourLeaves.size();
	size_t endLeafIndexS3 = subtree3.second % eulerTourLeaves.size();

	updateQuartetsThreeClades(startLeafIndexS1, endLeafIndexS1, startLeafIndexS2, endLeafIndexS2, startLeafIndexS3,
			endLeafIndexS3, eulerTourLeaves, t1, t2);
	updateQuartetsThreeClades(startLeafIndexS2, endLeafIndexS2, startLeafIndexS1, endLeafIndexS1, startLeafIndexS3,
			endLeafIndexS3, eulerTourLeaves, t1, t2);
	updateQuartetsThreeClades(startLeafIndexS3, endLeafIndexS3, startLeafIndexS1, endLeafIndexS1, startLeafIndexS2,
			endLeafIndexS2, eulerTourLeaves, t1, t2);
}

/**
 * An inner node in a bifurcating tree induces three subtrees S_1, S_2, and S_3.
 * Given an evaluation tree and an inner node, update the quartet topology counts of all quartets
 * {a,b,c,d} for which a and b are in the same subtree, c is in another subtree, and d is in the remaining subtree.
 * @param tree the evaluation tree
 * @param nodeIdx ID of an inner node in the evaluation tree
 * @param eulerTourLeaves the leaves' IDs of the tree traversed in an euler tour order
 * @param linkToEulerLeafIndex Mapping of each link in the tree to indices in the euler tour;
 * 	needed for determining first and last index of leaves belonging to a subtree.
 */
template<typename CINT>
void QuartetCounterLookup<CINT>::updateQuartets(const Tree &tree, size_t nodeIdx,
		const std::vector<int> &eulerTourLeaves, const std::vector<int> &linkToEulerLeafIndex, int t1, int t2) {
	// get taxa from subtree clades at nodeIdx
	std::vector<size_t> subtreeLinkIndices;
	const TreeLink* actLinkPtr = &tree.node_at(nodeIdx).link();
	subtreeLinkIndices.push_back(actLinkPtr->index());
	while (subtreeLinkIndices[0] != actLinkPtr->next().index()) {
		actLinkPtr = &actLinkPtr->next();
		subtreeLinkIndices.push_back(actLinkPtr->index());
	}

	for (size_t i = 0; i < subtreeLinkIndices.size(); ++i) {
		for (size_t j = i + 1; j < subtreeLinkIndices.size(); ++j) {
			for (size_t k = j + 1; k < subtreeLinkIndices.size(); ++k) {
				size_t link1 = subtreeLinkIndices[i];
				size_t link2 = subtreeLinkIndices[j];
				size_t link3 = subtreeLinkIndices[k];
				updateQuartetsThreeLinks(link1, link2, link3, tree, eulerTourLeaves, linkToEulerLeafIndex, t1, t2);
			}
		}
	}
}

/**
 * Fill the lookup table by counting quartet topologies in the set of evaluation trees.
 * @param evalTreesPath path to the file containing the set of evaluation trees
 * @param m number of evaluation trees
 * @param taxonToReferenceID mapping of taxon names to leaf ID in reference tree
 */
template<typename CINT>
void QuartetCounterLookup<CINT>::countQuartets(const std::string &evalTreesPath, size_t m,
		const std::unordered_map<std::string, size_t> &taxonToReferenceID,
		const uint64_t max_em_elements
	) {
	unsigned int progress = 1;
	const auto onePercent = m / 200;

	utils::InputStream instream(utils::make_unique<utils::FileInputSource>(evalTreesPath));
//	auto itTree = NewickInputIterator(instream, DefaultTreeNewickReader());
	size_t i = 0;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point end;	


	short thread = 0;
	if(2<=(nthread/2)) thread = floor(nthread/2);
	else thread = nthread; 

#pragma omp parallel num_threads(thread)     
#pragma omp single
	{                   
		stxxl::stats* Stats = stxxl::stats::get_instance();
		stxxl::stats_data stats_begin(*Stats);
	for(auto itTree = NewickInputIterator(instream, DefaultTreeNewickReader()); itTree; ++itTree) { // iterate over the set of evaluation trees
		auto tree = *itTree;
		#pragma omp task firstprivate(tree)
		{		
        	int tid1 = omp_get_thread_num();
		std::cout << "tid1: " << tid1 << std::endl;

		size_t nEval = tree.node_count();

		// do an euler tour through the tree
		std::vector<int> eulerTourLeaves; // directly containing the mapped IDs from the reference
		std::vector<int> linkToEulerLeafIndex;
		linkToEulerLeafIndex.resize(tree.link_count());
		for (auto it : eulertour(tree)) {
			if (it.node().is_leaf()) {
				size_t leafIdx = it.node().index();
				eulerTourLeaves.push_back(
						refIdToLookupID[taxonToReferenceID.at(tree.node_at(leafIdx).data<DefaultNodeData>().name)]);
			}
			linkToEulerLeafIndex[it.link().index()] = eulerTourLeaves.size();
		}
	short thread = 0;
	if(2<=(nthread/2)) thread = 2;
	else thread = 1; 
			
#pragma omp parallel num_threads(nthread)	
	 {			
	 	int tid2 = omp_get_thread_num();
		std::cout << "tid2: " << tid2 <<std::endl;
#pragma omp for schedule(dynamic)
			for (size_t j = 0; j < nEval; ++j) {
				if (!tree.node_at(j).is_leaf()) {
					updateQuartets(tree, j, eulerTourLeaves, linkToEulerLeafIndex, tid1, tid2);
				}
			}

		if (i > progress * onePercent) {
			std::cout << "Counting quartets... " << progress << "%" << std::endl;
			progress++;
		}
	}//end parallel leaves
	if (quartetSorter[tid1].size() > max_em_elements/nthread*2) {
		end = std::chrono::steady_clock::now();
		LOG(INFO) << "[counting_time] [" << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()<< " µs]";
		begin = std::chrono::steady_clock::now();
		quartetSorter[tid1].sort();
		end = std::chrono::steady_clock::now();
		LOG(INFO) << "[sorting_time] [" << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()<< " µs]";
		#pragma omp critical
		{	
			begin = std::chrono::steady_clock::now();
			quartetCount.update(quartetSorter[tid1]);
			quartetSorter[tid1].clear();
			end = std::chrono::steady_clock::now();
			LOG(INFO) << "[readingSorter_time] [" << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()<< " µs]";
			begin = std::chrono::steady_clock::now();
		}
	}
		++i;
	}//pragma omp task
#pragma omp taskwait
	}//end parallel treeLoop		
		//++itTree;
	/*if (quartetSorter[tid1].size()) {
		end = std::chrono::steady_clock::now();
		LOG(INFO) << "[counting_time] [" << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()<< " µs]";
		begin = std::chrono::steady_clock::now();
		quartetSorter[tid1].sort();
		end = std::chrono::steady_clock::now();
		LOG(INFO) << "[sorting_time] [" << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()<< " µs]";
		#pragma omp critical
		{
			begin = std::chrono::steady_clock::now();
			quartetCount.update(quartetSorter[tid1]);
			quartetSorter[tid1].clear();
			end = std::chrono::steady_clock::now();
			LOG(INFO) << "[readingSorter_time] [" << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()<< " µs]";
		}
	}
	*/
	stxxl::stats_data stats_end(*Stats);
	LOG(INFO) << "[run_volumeWritten] [" << (stats_end - stats_begin).get_written_volume ()<< " bytes]"; 
	std::cout << (stxxl::stats_data(*Stats) - stats_begin); // print i/o statistics
	}//end parallel region
	
}

/**
 * @param refTree the reference tree
 * @param evalTreesPath path to the file containing the set of evaluation trees
 * @param m the number of evaluation trees
 */
template<typename CINT>
QuartetCounterLookup<CINT>::QuartetCounterLookup(Tree const &refTree, const std::string &evalTreesPath, size_t m, bool verboseOutput,
		bool savemem,int num_threads, int internalMemory) :
		savemem(savemem){
//quartetSorter(my_comparator<uint64_t>(),static_cast<size_t>(1)<<internalMemory, num_threads) {
//quartetSorter(my_comparator<size_t>(),static_cast<size_t>(1)<<32) {
	std::unordered_map<std::string, size_t> taxonToReferenceID;
	refIdToLookupID.resize(refTree.node_count());
	lookupIdToRefId.resize(refTree.node_count());
	nthread = num_threads;	
	n = 0;
	if(2<=(num_threads/2)){
		for(short i =0; i < num_threads; i+2){
			sorter_type q_sorter(my_comparator<uint64_t>(),static_cast<size_t>(1)<< static_cast<int>((internalMemory-floor(num_threads/4))), 2);
			quartetSorter.push_back(q_sorter);
		}
	}
	else{
		sorter_type q_sorter(my_comparator<uint64_t>(),static_cast<size_t>(1)<<internalMemory, num_threads);
		quartetSorter.push_back(q_sorter);
	}
	//TIMED_BLOCK(timerObj, "QuartetCounterLookup_time"){

	for (auto it : eulertour(refTree)) {
		if (it.node().is_leaf()) {
			taxonToReferenceID[it.node().data<DefaultNodeData>().name] = it.node().index();
			refIdToLookupID[it.node().index()] = n;
			lookupIdToRefId[n] = (size_t)it.node().index();
			n++;
		}
	}

	qsc = make_unique<QuartetScoreComputer<CINT>> (refTree, evalTreesPath, m, verboseOutput, savemem, num_threads, internalMemory, refIdToLookupID);
	countQuartets(evalTreesPath, m, taxonToReferenceID);
	qsc->initScores();
	reduceSorter();
	//std::tuple <size_t,size_t,size_t,size_t,std::array<CINT,3>,IT> result;
	//auto& quartetCountIterator = *quartetCount;
		
	//while(!quartetCount.empty()){

		//result = reduceSorter(quartetCountIterator);
		//quartetCountIterator = std::get<5>(result);
		//qsc->computeQuartetScoresBifurcatingQuartets(std::get<0>(result), std::get<1>(result), std::get<2>(result), std::get<3>(result), std::get<4>(result));
	//}
	//};//TIMED_BLOCK
}

/**
 * Returns the count of the quartet topology ab|cd in the evaluation trees... only needed for the fast option
 * @param aIdx ID of taxon a
 * @param bIdx ID of taxon b
 * @param cIdx ID of taxon c
 * @param dIdx ID of taxon d
 */
template<typename CINT>
CINT QuartetCounterLookup<CINT>::lookupQuartetCount(size_t aIdx, size_t bIdx, size_t cIdx, size_t dIdx) const {
	aIdx = refIdToLookupID[aIdx];
	bIdx = refIdToLookupID[bIdx];
	cIdx = refIdToLookupID[cIdx];
	dIdx = refIdToLookupID[dIdx];
	return lookupTableFast[CO(aIdx, bIdx, cIdx, dIdx)] + lookupTableFast[CO(aIdx, bIdx, dIdx, cIdx)]
			+ lookupTableFast[CO(bIdx, aIdx, cIdx, dIdx)] + lookupTableFast[CO(bIdx, aIdx, dIdx, cIdx)];
}

/**
 * Returns the counts of the quartet topologies ab|cd, ac|bd, and ad|bc in the evaluation trees
 * @param aIdx ID of taxon a
 * @param bIdx ID of taxon b
 * @param cIdx ID of taxon c
 * @param dIdx ID of taxon d
 */
template<typename CINT>
std::tuple<CINT, CINT, CINT> QuartetCounterLookup<CINT>::countQuartetOccurrences(size_t aIdx, size_t bIdx, size_t cIdx,
		size_t dIdx) const {
	if (savemem) {
		size_t a = refIdToLookupID[aIdx];
		size_t b = refIdToLookupID[bIdx];
		size_t c = refIdToLookupID[cIdx];
		size_t d = refIdToLookupID[dIdx];
		const auto& tuple = lookupTable.get_tuple(a, b, c, d);
		CINT abCD = tuple[lookupTable.tuple_index(a, b, c, d)];
		CINT acBD = tuple[lookupTable.tuple_index(a, c, b, d)];
		CINT adBC = tuple[lookupTable.tuple_index(a, d, b, c)];
		return std::tuple<CINT, CINT, CINT>(abCD, acBD, adBC);
	} else {
		CINT abCD = lookupQuartetCount(aIdx, bIdx, cIdx, dIdx);
		CINT acBD = lookupQuartetCount(aIdx, cIdx, bIdx, dIdx);
		CINT adBC = lookupQuartetCount(aIdx, dIdx, bIdx, cIdx);
		return std::tuple<CINT, CINT, CINT>(abCD, acBD, adBC);
	}
}

template<typename CINT>
void QuartetCounterLookup<CINT>::reduceSorter() {
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point end;

	constexpr uint64_t MASK_TUPLE_INDEX = 0x3;
	constexpr uint64_t MASK_QUARTET_INDEX = ~MASK_TUPLE_INDEX;

	uint64_t totalNumberTaxaCounts = 0;
	
	auto commit = [this] (size_t qi, const std::array<CINT, 3> q123)  {
		const auto quartet = qsc->get_leaves(qi);
		const auto a = lookupIdToRefId[quartet[0]];
		const auto b = lookupIdToRefId[quartet[1]];
		const auto c = lookupIdToRefId[quartet[2]];
		const auto d = lookupIdToRefId[quartet[3]];

		qsc->computeQuartetScoresBifurcatingQuartets(a,b,c,d,q123);
		//size_t tuple = lookupTable.get_index(quartet[0], quartet[1], quartet[2], quartet[3]);
	};

	quartetCount.rewind();
	if (quartetCount.empty()) {
		std::cerr << "No Quartets found\n";
		abort();
	}

#pragma omp parallel num_threads(nthread)
{
	#pragma omp single
	{
	std::array<CINT, 3> q123;
	q123.fill(0);
	size_t last_qi = (*quartetCount).first & MASK_QUARTET_INDEX;

	for(; !quartetCount.empty(); ++quartetCount) {
		const auto& item = *quartetCount;
		//const auto item = begin;

		const auto qi = item.first & MASK_QUARTET_INDEX;
		const auto ti = item.first & MASK_TUPLE_INDEX;

		if (qi == last_qi) {
			assert(ti < 4);
		} else {
			#pragma omp task
			commit(last_qi, q123);
			last_qi = qi;
			q123.fill(0);
		}

		q123[ti] = item.second;
		totalNumberTaxaCounts += item.second;
	}
	commit(last_qi, q123);
	#pragma omp taskwait
	}
}


	qsc->calculateQPICScores();
	//quartetSorter.clear();
	end = std::chrono::steady_clock::now();
	LOG(INFO) << "[computing_time] [" << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()<< " µs]";
	LOG(INFO) << "[run_totalNumberTaxaCounts] [" << totalNumberTaxaCounts << " counts]";
}
