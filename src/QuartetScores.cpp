#include "genesis/genesis.hpp"
#include "quartet_newick_writer.hpp"
#include "QuartetCounterLookup.hpp"
#include "tclap/CmdLine.h" // command line parser, downloaded from http://tclap.sourceforge.net/
#include "easylogging++.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>

#ifdef GENESIS_OPENMP
#	include <omp.h>
#endif

using namespace genesis;
using namespace tree;

INITIALIZE_EASYLOGGINGPP

/**
 * Count the number of evaluation trees.
 * @param evalTreesPath path to the file containing the set of evaluation trees
 */
size_t countEvalTrees(const std::string &evalTreesPath) {
	size_t count = 0;
	utils::InputStream instream(utils::make_unique<utils::FileInputSource>(evalTreesPath));
	auto it = NewickInputIterator(instream);
	while (it) {
		count++;
		++it;
	}
	return count;
}

/**
 * The main method. Compute quartet scores and store the result in a tree file.
 */
int main(int argc, char* argv[]) {
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	bool verbose = false;
	bool savemem = false;
	std::string pathToReferenceTree;
	std::string pathToEvaluationTrees;
	std::string outputFilePath;
	size_t nThreads = 0;
	int internalMemory = 33;

    // Load configuration from file
    el::Configurations conf("../logging.conf");
    // Actually reconfigure all loggers
    el::Loggers::reconfigureAllLoggers(conf);

	try {
		TCLAP::CmdLine cmd("Compute quartet scores", ' ', "1.0");
		TCLAP::ValueArg<std::string> refArg("r", "ref", "Path to the reference tree", true, "", "string");
		TCLAP::ValueArg<std::string> evalArg("e", "eval", "Path to the evaluation trees", true, "", "string");
		TCLAP::ValueArg<std::string> outputArg("o", "output", "Path to the output file", true, "", "string");
		TCLAP::ValueArg<size_t> threadsArg("t", "threads", "Maximum number of threads to use", false, 0, "uint");
		TCLAP::ValueArg<int> intMemArg("i", "internal", "Internal memory to use for external structure", false, 33, "uint");
		TCLAP::SwitchArg verboseArg("v", "verbose", "Verbose mode", false);
		TCLAP::SwitchArg savememArg("s", "savemem", "Consume less memory, but with the cost of increased runtime", false);
		cmd.add(refArg);
		cmd.add(evalArg);
		cmd.add(outputArg);
		cmd.add(intMemArg);
		cmd.add(threadsArg);
		cmd.add(verboseArg);
		cmd.add(savememArg);
		cmd.parse(argc, argv);

		pathToReferenceTree = refArg.getValue();
		pathToEvaluationTrees = evalArg.getValue();
		outputFilePath = outputArg.getValue();
		nThreads = threadsArg.getValue();
		internalMemory = intMemArg.getValue();
		verbose = verboseArg.getValue();
		savemem = savememArg.getValue();
	} catch (TCLAP::ArgException &e) // catch any exceptions
	{
		std::cerr << "ERROR: " << e.error() << " for arg " << e.argId() << std::endl;
		return 1;
	}

	std::ifstream infile(outputFilePath);
	if (infile.good()) {
		std::cout << "ERROR: The specified output file already exists.\n";
		return 1;
	}

	if (nThreads > 0) {
		#ifdef GENESIS_OPENMP
			omp_set_num_threads(nThreads);
		#else
		    std::cerr << "Warning: You specified to use multiple threads, but compiled without OpenMP. "
			          << "Thus, we can only use one thread." << std::endl;
		#endif
	}

	//read trees
	DefaultTreeNewickReader reader;
	Tree referenceTree = reader.from_file(pathToReferenceTree);

	if (verbose) {
		auto tp = PrinterCompact();
		auto res = tp.print(referenceTree, []( TreeNode const& node, TreeEdge const& edge ) {
			//return node.data<DefaultNodeData>().name + " edge: " + std::to_string(edge.index()) + " vertex: " + std::to_string(node.index());
				(void) edge;
				return node.data<DefaultNodeData>().name + " " + std::to_string( node.index() );
			});

		std::cout << res << std::endl;
	}

	std::vector<double> lqic;
	std::vector<double> qpic;
	std::vector<double> eqpic;
	size_t m = countEvalTrees(pathToEvaluationTrees);
	m = 2*m;
	if (m < (size_t(1) << 8)) {
		QuartetCounterLookup<uint8_t> qcl(referenceTree, pathToEvaluationTrees, m, verbose, savemem, nThreads, internalMemory);
		lqic = qcl.qsc->getLQICScores();
		qpic = qcl.qsc->getQPICScores();
		eqpic = qcl.qsc->getEQPICScores();
	} else if (m < (size_t(1) << 16)) {
		QuartetCounterLookup<uint16_t> qcl(referenceTree, pathToEvaluationTrees, m, verbose, savemem, nThreads, internalMemory);
		lqic = qcl.qsc->getLQICScores();
		qpic = qcl.qsc->getQPICScores();
		eqpic = qcl.qsc->getEQPICScores();
	} else if (m < (size_t(2) << 32)) {
		QuartetCounterLookup<uint32_t> qcl(referenceTree, pathToEvaluationTrees, m, verbose, savemem, nThreads, internalMemory);
		lqic = qcl.qsc->getLQICScores();
		qpic = qcl.qsc->getQPICScores();
		eqpic = qcl.qsc->getEQPICScores();
	} else {
		QuartetCounterLookup<uint64_t> qcl(referenceTree, pathToEvaluationTrees, m, verbose, savemem, nThreads, internalMemory);
		lqic = qcl.qsc->getLQICScores();
		qpic = qcl.qsc->getQPICScores();
		eqpic = qcl.qsc->getEQPICScores();
	}
/*
	std::ofstream output;
	output.open("qpic_scores.csv");
	for(size_t i = 0; i < qpic.size(); i++){
		output << qpic[i] << std::endl;
	}
	output.close();
	output.open("lqic_scores.csv");
	for(size_t i = 0; i < lqic.size(); i++){
		output << lqic[i] << std::endl;
	}
	output.close();
	output.open("eqpic_scores.csv");
	for(size_t i = 0; i < eqpic.size(); i++){
		output << eqpic[i] << std::endl;
	}
	output.close();
*/
	// Create the writer and assign values.
	auto writer = QuartetTreeNewickWriter();
	writer.set_lq_ic_scores(lqic);
	if (!eqpic.empty()) { // bifurcating tree
		writer.set_eqp_ic_scores(eqpic);
	}
	if (!qpic.empty()) { // bifurcating tree
		writer.set_qp_ic_scores(qpic);
	}

	writer.to_file(referenceTree, outputFilePath);
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	LOG(INFO) << "[total_time] [" << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()<< " µs]";

	return 0;
}
