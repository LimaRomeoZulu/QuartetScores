#pragma once

#include "genesis/genesis.hpp"
#include <vector>
#include <cassert>
#include <algorithm>
#include <memory>
#include "TreeInformation.hpp"
#include "quartet_lookup_table.hpp"
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
public:
	QuartetCounterLookup(const Tree &refTree, const std::string &evalTreesPath, size_t m, bool savemem, int num_threads, int internalMemory);
	~QuartetCounterLookup() = default;
	std::tuple<CINT, CINT, CINT> countQuartetOccurrences(size_t aIdx, size_t bIdx, size_t cIdx, size_t dIdx) const;
private:
	void countQuartets(const std::string &evalTreesPath, size_t m,
			const std::unordered_map<std::string, size_t> &taxonToReferenceID);
	void updateQuartets(const Tree &tree, size_t nodeIdx, const std::vector<int> &eulerTourLeaves,
			const std::vector<int> &linkToEulerLeafIndex, int t);
	void updateQuartetsThreeLinks(size_t link1, size_t link2, size_t link3, const Tree &tree,
			const std::vector<int> &eulerTourLeaves, const std::vector<int> &linkToEulerLeafIndex, int t);
	void updateQuartetsThreeClades(size_t startLeafIndexS1, size_t endLeafIndexS1, size_t startLeafIndexS2,
			size_t endLeafIndexS2, size_t startLeafIndexS3, size_t endLeafIndexS3,
			const std::vector<int> &eulerTourLeaves, int t);
	std::pair<size_t, size_t> subtreeLeafIndices(size_t linkIdx, const Tree &tree,
			const std::vector<int> &linkToEulerLeafIndex);

	CINT lookupQuartetCount(size_t aIdx, size_t bIdx, size_t cIdx, size_t dIdx) const;

	std::vector<CINT> lookupTableFast; /**> larger O(n^4) lookup table storing the count of each quartet topology */
	QuartetLookupTable<CINT> lookupTable; /**> smaller O(n^4) lookup table storing the count of each quartet topology */

	size_t n; /**> number of taxa in the reference tree */
	size_t n_square; /**> n*n */
	size_t n_cube; /**> n*n*n */
	std::vector<size_t> refIdToLookupID;
	bool savemem; /**> trade speed for less memory or not */
	void reduceSorter();
	stxxl::parallel_sorter_synchron<uint64_t, my_comparator<uint64_t> > quartetSorter;
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
		const std::vector<int> &eulerTourLeaves, int t) {
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
size_t tmp = CO(a,a2,b,c);
quartetSorter.push(tmp,t);	
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
		const std::vector<int> &eulerTourLeaves, const std::vector<int> &linkToEulerLeafIndex, int t) {
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
			endLeafIndexS3, eulerTourLeaves, t);
	updateQuartetsThreeClades(startLeafIndexS2, endLeafIndexS2, startLeafIndexS1, endLeafIndexS1, startLeafIndexS3,
			endLeafIndexS3, eulerTourLeaves, t);
	updateQuartetsThreeClades(startLeafIndexS3, endLeafIndexS3, startLeafIndexS1, endLeafIndexS1, startLeafIndexS2,
			endLeafIndexS2, eulerTourLeaves, t);
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
		const std::vector<int> &eulerTourLeaves, const std::vector<int> &linkToEulerLeafIndex, int t) {
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
				updateQuartetsThreeLinks(link1, link2, link3, tree, eulerTourLeaves, linkToEulerLeafIndex, t);
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
		const std::unordered_map<std::string, size_t> &taxonToReferenceID) {
	unsigned int progress = 1;
	float onePercent = (float) m / 100;

	utils::InputStream instream(utils::make_unique<utils::FileInputSource>(evalTreesPath));
	auto itTree = NewickInputIterator(instream, DefaultTreeNewickReader());
	size_t i = 0;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point end;	


	stxxl::stats* Stats = stxxl::stats::get_instance();
	
	stxxl::stats_data stats_begin(*Stats);

	while (itTree) { // iterate over the set of evaluation trees
		TIMED_BLOCK(timerObj, "while(itTree)_time"){
		
		Tree const& tree = *itTree;

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
			
#pragma omp parallel num_threads(nthread)	
	  {			
	       int tid = omp_get_thread_num();
#pragma omp for schedule(dynamic)
		for (size_t j = 0; j < nEval; ++j) {
			if (!tree.node_at(j).is_leaf()) {
				updateQuartets(tree, j, eulerTourLeaves, linkToEulerLeafIndex, tid);
			}
		}

		if (i > progress * onePercent) {
			std::cout << "Counting quartets... " << progress << "%" << std::endl;
			progress++;
		}
		}
		};
		if((i!=0) && (i%250 == 0)){
			//end = std::chrono::steady_clock::now();
			//std::cout << "Counting took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()<< " microseconds." << std::endl;
			//reduceSorter();
			//begin = std::chrono::steady_clock::now();

		}
		++itTree;
		++i;
	}
	reduceSorter();
	
	stxxl::stats_data stats_end(*Stats);
	LOG(INFO) << "[run_volumeWritten] [" << (stats_end - stats_begin).get_written_volume ()<< " bytes]"; 
		
	std::cout << (stxxl::stats_data(*Stats) - stats_begin); // print i/o statistics
	//io_stats.close();
        //std::ofstream outputfile;
        //outputfile.open("output_Scores.csv");
        //for(size_t i = 0; i < lookupTableFast.size(); i++){
          //      outputfile << i << "," << lookupTableFast[i] << std::endl;
        //}
        //outputfile.close();
}

/**
 * @param refTree the reference tree
 * @param evalTreesPath path to the file containing the set of evaluation trees
 * @param m the number of evaluation trees
 */
template<typename CINT>
QuartetCounterLookup<CINT>::QuartetCounterLookup(Tree const &refTree, const std::string &evalTreesPath, size_t m,
		bool savemem,int num_threads, int internalMemory) :
		savemem(savemem),
quartetSorter(my_comparator<uint64_t>(),static_cast<size_t>(1)<<internalMemory, num_threads) {
//quartetSorter(my_comparator<size_t>(),static_cast<size_t>(1)<<32) {
	std::unordered_map<std::string, size_t> taxonToReferenceID;
	refIdToLookupID.resize(refTree.node_count());
	nthread = num_threads;	
	n = 0;
	TIMED_BLOCK(timerObj, "QuartetCounterLookup_time"){

	for (auto it : eulertour(refTree)) {
		if (it.node().is_leaf()) {
			taxonToReferenceID[it.node().data<DefaultNodeData>().name] = it.node().index();
			refIdToLookupID[it.node().index()] = n;
			n++;
		}
	}
	n_square = n * n;
	n_cube = n_square * n;
	// initialize the lookup table.
	if (savemem) {
		lookupTable.init(n);
	} else {
		lookupTableFast.resize(n * n * n * n);
	}
	countQuartets(evalTreesPath, m, taxonToReferenceID);
	if (savemem) {
		std::cout << "lookup table size in bytes: " << lookupTable.size() << "\n";
	} else {
		std::cout << "lookup table size in bytes: " << lookupTableFast.size() * sizeof(CINT) << "\n";
	}
	};
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
	TIMED_BLOCK(obj_s, "sorting_time"){
		quartetSorter.sort();
	}

	TIMED_BLOCK(obj_r, "readSorter_time"){
    	size_t tmp = *quartetSorter;
	CINT counter = 0;
    for(;!quartetSorter.empty();++quartetSorter)
    {
		if(tmp == *quartetSorter){
			counter++;
		}
		else{
			lookupTableFast[tmp] = lookupTableFast[tmp] + counter;
			counter = 1;
			tmp = *quartetSorter;
		}
    }
	lookupTableFast[tmp] = lookupTableFast[tmp] + counter;
	quartetSorter.clear();
	}
}
