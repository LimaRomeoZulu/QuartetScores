#pragma once

//#define USE_STXXL

#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

#ifdef USE_STXXL
#include <stxxl/vector>
#endif

// =================================================================================================
//     Metaquartet Lookup Table
// =================================================================================================

template<typename LookupIntType>
class MetaquartetLookupTable {
public:

	// -------------------------------------------------------------------------
	//     Typedefs and Enums
	// -------------------------------------------------------------------------

	using QuartetTuple = std::array< LookupIntType, 3 >;

	// -------------------------------------------------------------------------
	//     Constructors and Rule of Five
	// -------------------------------------------------------------------------

	MetaquartetLookupTable() :
			num_taxa_(0) {
	}

	MetaquartetLookupTable(size_t num_taxa) {
		init(num_taxa);
	}

	~MetaquartetLookupTable() {
#ifdef USE_STXXL
		while (!quartet_lookup_.empty()) {
			quartet_lookup_.pop_back();
		}
#endif
	}

	MetaquartetLookupTable(MetaquartetLookupTable const&) = default;
	MetaquartetLookupTable(MetaquartetLookupTable&&) = default;

	MetaquartetLookupTable& operator=(MetaquartetLookupTable const&) = default;
	MetaquartetLookupTable& operator=(MetaquartetLookupTable&&) = default;

	// -------------------------------------------------------------------------
	//     Public Interface
	// -------------------------------------------------------------------------

	void init(size_t num_taxa) {
		num_taxa_ = num_taxa;
		// init_binom_lookup_(num_taxa);
		init_meta_quartet_lookup_(num_taxa);
	}

	size_t num_taxa() const {
		return num_taxa_;
	}

	size_t size() const {
		return (quartet_lookup_.size() * 3 * sizeof(LookupIntType)) + (binom_lookup_.size() + 1) * sizeof(size_t);
	}

	std::tuple<int,int,int,int> get_leaves(uint64_t q){
		uint64_t mask = 32767;
		//get all possible inner nodes u
		int a = static_cast<int>((q & (mask << 49)) >> 49);
		int b = static_cast<int>((q & (mask << 34)) >> 34);
		//get all possible inner nodes v
		int c = static_cast<int>((q & (mask << 19)) >> 19);
		int d = static_cast<int>((q & (mask << 4)) >> 4);
		
		return std::tuple<int,int,int,int>(a,b,c,d);
	}

	uint64_t get_index(size_t a, size_t b, size_t c, size_t d) const {
		uint64_t tmp = lookup_index_(a,b,c,d);
		return tmp;
	}

	short get_tuple(size_t a, size_t b, size_t c, size_t d){
		return tuple_index_(a,b,c,d);
	}


	// -------------------------------------------------------------------------
	//     Private Members
	// -------------------------------------------------------------------------


	void init_meta_quartet_lookup_(size_t num_taxa) {
		// calculate ncr(n, 4)
		//size_t const n = (num_taxa * (num_taxa - 1)) / 2;
		//quartet_lookup_ = std::vector<QuartetTuple>(n, {{ 0, 0, 0 }});
	}

	size_t get_combination_id_(int a, int b){
		size_t result = 0;
		if(a > b){
			result = (a*(a-1)/2) + b;
		}
		else{
			result = (b*(b-1)/2) + a;
		}
		return result;
	}

	uint64_t bit_shifting_index_(size_t a, size_t b, size_t c, size_t d, size_t tupleIndex) const {
		uint64_t res = 0;
		uint64_t tmp = 0;
		size_t quartet_id [4] = {a,b,c,d};
		for (short i = 1; i < 5; i++){
			tmp = 0;
			tmp = quartet_id[i-1] << (64-(i*15));
			res += tmp;
		}
		res += tupleIndex;

		return res;
	}

	short tuple_index_(size_t a, size_t b, size_t c, size_t d) const {
		// Get all comparisons that we need.
		bool const ac = (a<c);
		bool const ad = (a<d);
		bool const bc = (b<c);
		bool const bd = (b<d);

		// Check first and third case. Second one is implied.
		bool const x = ((ac) & (ad) & (bc) & (bd)) | ((!ac) & (!bc) & (!ad) & (!bd));
		bool const ab_in_cd = ((!ac) & (ad) & (!bc) & (bd)) | ((!ad) & (ac) & (!bd) & (bc));
		bool const cd_in_ab = ((ac) & (!bc) & (ad) & (!bd)) | ((bc) & (!ac) & (bd) & (!ad));
		bool const z = ab_in_cd | cd_in_ab;
		bool const y = !x & !z;

		// Only one can be set.
		assert(!(x & y & z));
		assert(x ^ y ^ z);
		assert(x | y | z);
		short const r = static_cast<short>(y) + 2 * static_cast<short>(z);

		// Result has to be fitting.
		assert(r < 3);
		assert((x && !y && !z && r == 0) || (!x && y && !z && r == 1) || (!x && !y && z && r == 2));
		return r;
	}

	uint64_t lookup_index_(size_t a, size_t b, size_t c, size_t d) const {
		size_t ta, tb, tc, td; // from largest to smallest
		size_t low1, high1, low2, high2, middle1, middle2;

		if (a < b) {
			low1 = a;
			high1 = b;
		} else {
			low1 = b;
			high1 = a;
		}
		if (c < d) {
			low2 = c;
			high2 = d;
		} else {
			low2 = d;
			high2 = c;
		}

		if (low1 < low2) {
			td = low1;
			middle1 = low2;
		} else {
			td = low2;
			middle1 = low1;
		}
		if (high1 > high2) {
			ta = high1;
			middle2 = high2;
		} else {
			ta = high2;
			middle2 = high1;
		}
		if (middle1 < middle2) {
			tc = middle1;
			tb = middle2;
		} else {
			tc = middle2;
			tb = middle1;
		}
		size_t tupleIndex = tuple_index_(a,b,c,d);
		return bit_shifting_index_(ta, tb, tc, td, tupleIndex);
	}


	// -------------------------------------------------------------------------
	//     Data Members
	// -------------------------------------------------------------------------

#ifdef USE_STXXL
	stxxl::VECTOR_GENERATOR<QuartetTuple>::result quartet_lookup_;
#else
	std::vector<QuartetTuple> quartet_lookup_;
#endif

	std::vector<size_t> binom_lookup_;

	size_t num_taxa_;
	std::vector<std::vector<uint64_t>> furcation_lookup_;

};
