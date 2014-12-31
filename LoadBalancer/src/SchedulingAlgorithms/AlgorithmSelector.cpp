/////////////////////////////////////////////////////////////////////
//  AlgorithmSelector.cpp - implementation of algorithm selector   
//  ver 1.0                                                        
//  Language:      standard C++ 11                               
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#include "../../include/SchedulingAlgorithms/SchedAlgorithms.h"

//-------------------------------------------------------------------
// Constructor
// Initialize scheduling algorithm type, and scheduling algorithm
// pointer. Now the pointer is a null pointer, and needed be casted
// to appropriate type in selectAlgorithm() function.
//-------------------------------------------------------------------
AlgorithmSelector::AlgorithmSelector(SchedAlgorithm sched_type)
	: sched_type_(sched_type_), sched_algo_(nullptr) 
{}

//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------
AlgorithmSelector::~AlgorithmSelector()
{
	delete sched_algo_;
}

//-------------------------------------------------------------------
// Set scheduling map
// Every time, invoke selectServer() function, sched_map needed be
// assure of the newest version.
//-------------------------------------------------------------------
void AlgorithmSelector::setSchedMap(const SchedMap& sched_map)
{
	sched_algo_->setSchedMap(sched_map);
}

//-------------------------------------------------------------------
// Set scheduling algorithm type
//-------------------------------------------------------------------
void AlgorithmSelector::setSchedType(const SchedAlgorithm sched_type) 
{ 
	sched_type_ = sched_type; 
}

//-------------------------------------------------------------------
// Get scheduling algorithm type
//-------------------------------------------------------------------
const SchedAlgorithm AlgorithmSelector::getSchedType()
{ 
	return sched_type_; 
}

//-------------------------------------------------------------------
// Set handle_ip, which is used in Destination Hashing and Source
// Hashing algorithms.
//-------------------------------------------------------------------
void AlgorithmSelector::setHandleIP(const std::string& handle_ip)
{
	if (sched_type_ == SchedAlgorithm::Destination_Hashing ||
		sched_type_ == SchedAlgorithm::Source_Hashing)
		sched_algo_->setHandleIP(handle_ip);
}

//-------------------------------------------------------------------
// Get scheduling algorithm pointer
//-------------------------------------------------------------------
const AbstractSchedAlgorithms* AlgorithmSelector::getSchedAlgoPtr() 
{
	return sched_algo_; 
}

//-------------------------------------------------------------------
// Select appropriate server, actually invoke scheduling algorithm
// pointer's selectServer() function.
//-------------------------------------------------------------------
int AlgorithmSelector::selectServer() 
{ 
	return sched_algo_->selectServer(); 
}

//-------------------------------------------------------------------
// Cast scheduling algorithm pointer to appropriate type
//-------------------------------------------------------------------
void AlgorithmSelector::selectAlgorithm()
{
	switch (sched_type_)
	{
	case Round_Robin:
		sched_algo_ = new SchedRR();
		break;
	case Weighted_Round_Robin:
		sched_algo_ = new SchedWRR();
		break;
	case Least_Connection:
		sched_algo_ = new SchedLC();
		break;
	case Weighted_Least_Connection:
		sched_algo_ = new SchedWLC();
		break;
	case Destination_Hashing:
		sched_algo_ = new SchedDH();
		break;
	case Source_Hashing:
		sched_algo_ = new SchedSH();
		break;
	default:
		break;;
	}
}
