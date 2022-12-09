#include "atpg.h"

using namespace CoreNs;

// **************************************************************************
// Function   [ Atpg::getValStr ]
// Commentor  [ CAL ]
// Synopsis   [ usage: return string type of Value
//              in:    Value
//              out:   string of Value
//            ]
// Date       [ started 2020/07/04    last modified 2020/07/04 ]
// **************************************************************************
std::string Atpg::getValStr(Value val)
{
	std::string valStr;
	switch (val)
	{
		case H:
			valStr = "H";
			break;
		case L:
			valStr = "L";
			break;
		case D:
			valStr = "D";
			break;
		case B:
			valStr = "B";
			break;
		case X:
			valStr = "X";
			break;
		default:
			valStr = "Error";
			break;
	}
	return valStr;
}

// **************************************************************************
// Function   [ Atpg::clearOneGateFaultEffect ]
// Commentor  [ CAL ]
// Synopsis   [ usage:
//                Replace value of a gate from D/B to H/L.
//              in:    Gate
//              out:   void
//            ]
// Date       [ started 2020/07/04    last modified 2020/07/04 ]
// **************************************************************************
void Atpg::clearOneGateFaultEffect(Gate &g)
{
	if (g.v_ == D)
	{
		g.v_ = H;
	}
	else if (g.v_ == B)
	{
		g.v_ = L;
	}
}

// **************************************************************************
// Function   [ Atpg::clearAllFaultEffectBySimulation ]
// Commentor  [ CAL ]
// Synopsis   [ usage:
//                Clear all the fault effects before test generation for next
//                target fault.
//              in:    void
//              out:   void
//            ]
// Date       [ started 2020/07/04    last modified 2020/07/04 ]
// **************************************************************************
void Atpg::clearAllFaultEffectBySimulation()
{

	// Remove fault effects in input gates
	int numOfInputGate = cir_->npi_ + cir_->nppi_;
	for (int i = 0; i < numOfInputGate; ++i)
	{
		Gate &g = cir_->gates_[i];
		clearOneGateFaultEffect(g);
	}

	// Simulate the whole circuit ( gates were sorted by lvl_ in "cir_->gates_" )
	for (int i = 0; i < cir_->tgate_; ++i)
	{
		Gate &g = cir_->gates_[i];
		g.v_ = evaluationGood(g);
	}
}

// **************************************************************************
// Function   [ Atpg::testClearFaultEffect ]
// Commentor  [ CAL ]
// Synopsis   [ usage:
//                Test clearAllFaultEffectBySimulation for all faults
//              in:    void
//              out:   void
//            ]
// Date       [ started 2020/07/04    last modified 2020/07/04 ]
// **************************************************************************
void Atpg::testClearFaultEffect(FaultList &faultListToTest)
{
	for (auto it = faultListToTest.begin(); it != faultListToTest.end(); ++it)
	{
		GENERATION_STATUS result = patternGeneration((**it), false);
		clearAllFaultEffectBySimulation();
		for (int i = 0; i < cir_->tgate_; ++i)
		{
			Gate &g = cir_->gates_[i];
			if ((g.v_ == D) || (g.v_ == B))
			{
				std::cerr << "testClearFaultEffect found bug" << std::endl;
				std::cin.get();
			}
		}
	}
}

// **************************************************************************
// Function   [ Atpg::storeCurrentGateValue ]
// Commentor  [ CAL ]
// Synopsis   [ usage:
//                Store g.v_ to g.preV_
//              in:   void
//              out:  Count of values which change from H/L to the value which is not
//                    the same as preV_.
//            ]
// Date       [ started 2020/07/04    last modified 2020/07/04 ]
// **************************************************************************
int Atpg::storeCurrentGateValue()
{
	int numAssignedValueChanged = 0;
	for (int i = 0; i < cir_->tgate_; ++i)
	{
		Gate &g = cir_->gates_[i];
		if ((g.preV_ != X) && (g.preV_ != g.v_))
		{
			numAssignedValueChanged++;
		}
		g.preV_ = g.v_;
	}
	if (numAssignedValueChanged != 0)
	{
		std::cout << "Bug: storeCurrentGateValue detects the numAssignedValueChanged is not 0." << std::endl;
		// std::cin.get();
	}
	return numAssignedValueChanged;
}

// **************************************************************************
// Function   [ Atpg::storeCurrentGateValue ]
// Commentor  [ CAL ]
// Synopsis   [ usage:
//                Calculate the depthFromPo_ of each gate.
//              notes:
//                If there is no path from a gate to PO/PPO, then I will
//                set its depthFromPo_ as cir_->tlvl_ + 100
//              in:    void
//              out:   void
//            ]
// Date       [ started 2020/07/06    last modified 2020/07/06 ]
// **************************************************************************
void Atpg::calDepthFromPo()
{
	int tlvlAddPlus100 = cir_->tlvl_ + 100;
	for (int i = 0; i < cir_->tgate_; ++i)
	{
		Gate &g = cir_->gates_[i];
		/*
		 * Because the default is -1, I want to check if it was changed or not.
		 * */
		if (g.depthFromPo_ != -1)
		{
			std::cout << "depthFromPo_ is not -1 " << std::endl;
			std::cin.get();
		}
	}

	// Update depthFromPo_ form PO/PPO to PI/PPI
	for (int i = cir_->tgate_ - 1; i >= 0; --i)
	{
		Gate &g = cir_->gates_[i];
		if ((g.type_ == Gate::PO) || (g.type_ == Gate::PPO))
		{
			g.depthFromPo_ = 0;
		}
		else if (g.nfo_ > 0)
		{
			// This output gate does not exist a path to PO/PPO
			if (cir_->gates_[g.fos_[0]].depthFromPo_ == tlvlAddPlus100)
			{
				g.depthFromPo_ = tlvlAddPlus100;
			}
			else
			{
				g.depthFromPo_ = cir_->gates_[g.fos_[0]].depthFromPo_ + 1;
			}

			for (int j = 1; j < g.nfo_; j++)
			{
				Gate &go = cir_->gates_[g.fos_[j]];
				if (go.depthFromPo_ < g.depthFromPo_)
				{
					g.depthFromPo_ = go.depthFromPo_ + 1;
				}
			}
		}
		else
		{
			/* Assign a value greater than maximal lvl_ as our default */
			g.depthFromPo_ = tlvlAddPlus100;
		}
	}

	// for ( int i = 0 ; i < cir_->tgate_ ; ++i ) {
	//   Gate &g = cir_->gates_[i];
	//   std::cout << "g.depthFromPo_ is " << g.depthFromPo_ << std::endl;
	// }
	// std::cin.get();
}

// **************************************************************************
// Function   [ Atpg::resetPreValue ]
// Commentor  [ CAL ]
// Synopsis   [ usage:
//                Reset the preV_ of each gate to X.
//              in:    void
//              out:   void
//            ]
// Date       [ started 2020/07/07    last modified 2020/07/07 ]
// **************************************************************************
void Atpg::resetPreValue()
{
	for (int i = 0; i < cir_->tgate_; ++i)
	{
		Gate &g = cir_->gates_[i];
		g.preV_ = X;
	}
}

// **************************************************************************
// Function   [ Atpg::isExistXPath ]
// Commentor  [ CAL ]
// Synopsis   [ usage:
//                Used before patternGeneration
//                Return true if there is X-path.
//                Otherwise, return false.
//              in:    Gate pointer
//              out:   possible or not
//            ]
// Date       [ started 2020/07/07    last modified 2020/07/07 ]
// **************************************************************************
bool Atpg::isExistXPath(Gate *pGate)
{
	/* Clear the xPathStatus_ from target gate to PO/PPO */
	/* TO-DO
	 * This part can be implemented by event-driven method
	 * */
	for (int i = pGate->id_; i < cir_->tgate_; ++i)
	{
		xPathStatus_[i] = UNKNOWN;
	}

	return xPathTracing(pGate);
}

// **************************************************************************
// Function   [ Atpg::clearEventList_ ]
// Commentor  [ CAL ]
// Synopsis   [ usage:
//                Clear eventList_ and carefully reset modify_ and isInEventList_
//              in:    check isInEventList_ correctness
//              out:   void
//            ]
// Date       [ started 2020/07/07    last modified 2020/07/07 ]
// **************************************************************************
void Atpg::clearEventList_(bool isDebug)
{

	// pop and unmark
	for (int i = 0; i < cir_->tlvl_; ++i)
	{
		while (!eventList_[i].empty())
		{
			int gid = eventList_[i].top();
			eventList_[i].pop();
			modify_[gid] = false;
			isInEventList_[gid] = false;
		}
	}

	/*
	 * Because I expect that all gates in circuit must be unmark after the above
	 * for-loop.
	 * */
	if (isDebug == true)
	{
		for (int i = 0; i < cir_->tgate_; ++i)
		{
			if (isInEventList_[i] == true)
			{
				std::cout << "Warning clearEventList_ found unexpected behavior" << std::endl;
				isInEventList_[i] = false;
				std::cin.get();
			}
		}
	}
}

// **************************************************************************
// Function   [ Atpg::resetIsInEventList ]
// Commentor  [ CAL ]
// Synopsis   [ usage:
//                Set all element in isInEventList_ to false
//              in:    void
//              out:   void
//            ]
// Date       [ started 2020/07/07    last modified 2020/07/07 ]
// **************************************************************************
void Atpg::resetIsInEventList()
{
	std::fill(isInEventList_.begin(), isInEventList_.end(), false);
}

// **************************************************************************
// Function   [ Atpg::setValueAndRunImp ]
// Commentor  [ CAL ]
// Synopsis   [ usage:
//                Directly set the output of a gate to specific value and
//                run implication by event driven.
//              in:    void
//              out:   void
//            ]
// Date       [ started 2020/07/07    last modified 2020/07/07 ]
// **************************************************************************
void Atpg::setValueAndRunImp(Gate &g, Value val)
{
	clearEventList_(true);
	g.v_ = val;
	for (int i = 0; i < g.nfo_; ++i)
	{
		Gate &og = cir_->gates_[g.fos_[i]];
		if (isInEventList_[og.id_] == false)
		{
			eventList_[og.lvl_].push(og.id_);
			isInEventList_[og.id_] = true;
		}
	}

	// event-driven simulation
	for (int i = g.lvl_; i < cir_->tlvl_; ++i)
	{
		while (!eventList_[i].empty())
		{
			int gid = eventList_[i].top();
			eventList_[i].pop();
			isInEventList_[gid] = false;
			// current gate
			Gate &cg = cir_->gates_[gid];
			Value newValue = evaluationGood(cg);
			if (cg.v_ != newValue)
			{
				cg.v_ = newValue;
				for (int j = 0; j < cg.nfo_; ++j)
				{
					Gate &og = cir_->gates_[cg.fos_[j]];
					if (isInEventList_[og.id_] == false)
					{
						eventList_[og.lvl_].push(og.id_);
						isInEventList_[og.id_] = true;
					}
				}
			}
		}
	}
}

// **************************************************************************
// Function   [ Atpg::checkLevelInfo ]
// Commentor  [ CAL ]
// Synopsis   [ usage:
//                To check if the lvl_ of all the gates does not exceed cir_->tlvl_
//              in:    void
//              out:   void
//            ]
// Date       [ started 2020/07/07    last modified 2020/07/07 ]
// **************************************************************************
void Atpg::checkLevelInfo()
{
	for (int i = 0; i < cir_->tgate_; ++i)
	{
		Gate &g = cir_->gates_[i];
		if (g.lvl_ >= cir_->tlvl_)
		{
			std::cout << "checkLevelInfo found that at least one g.lvl_ is greater than cir_->tlvl_" << std::endl;
			std::cin.get();
		}
	}
}

// **************************************************************************
// Function   [ Atpg::StuckAtFaultATPGWithDTC ]
// Commentor  [ CAL ]
// Synopsis   [ usage: do stuck at fault model ATPG with dynamic test compression
//              in:    Pattern list, Fault list, int untest
//              out:   void //add pattern to PatternProcessor*
//            ]
// Date       [ started 2020/07/07    last modified 2021/09/14 ]
// **************************************************************************
void Atpg::StuckAtFaultATPGWithDTC(FaultList &faultListToGen, PatternProcessor *pcoll, int &untest)
{
	static int oriPatNum = 0;
	GENERATION_STATUS result = patternGeneration(*faultListToGen.front(), false);

	if (result == TEST_FOUND)
	{
		oriPatNum++;
		std::cout << oriPatNum << "-th pattern" << std::endl;
		Pattern *p = new Pattern;
		p->pi1_ = new Value[cir_->npi_];
		p->ppi_ = new Value[cir_->nppi_];
		p->po1_ = new Value[cir_->npo_];
		p->ppo_ = new Value[cir_->nppi_];
		pcoll->pats_.push_back(p);
		resetPreValue();
		clearAllFaultEffectBySimulation();
		storeCurrentGateValue();
		assignPatternPiValue(pcoll->pats_.back());

		if (pcoll->dynamicCompression_ == PatternProcessor::ON)
		{

			FaultList faultListTemp = faultListToGen;
			sim_->pfFaultSim(pcoll->pats_.back(), faultListToGen);
			assignPatternPoValue(pcoll->pats_.back());

			for (FaultListIter it = faultListTemp.begin(); it != faultListTemp.end(); ++it)
			{
				// skip detected faults
				if ((*it)->faultState_ == Fault::DT)
					continue;

				Gate *pGateForAtivation = getWireForActivation((**it));
				if (((pGateForAtivation->v_ == L) && ((*it)->faultType_ == Fault::SA0)) ||
						((pGateForAtivation->v_ == H) && ((*it)->faultType_ == Fault::SA1)))
				{
					continue;
				}

				// Activation check
				if (pGateForAtivation->v_ != X)
				{
					if (((*it)->faultType_ == Fault::SA0) || ((*it)->faultType_ == Fault::SA1))
					{
						setValueAndRunImp((*pGateForAtivation), X);
					}
					else
					{
						continue;
					}
				}

				if (isExistXPath(pGateForAtivation) == true)
				{
					// TO-DO homework 05 implement DTC here end of TO-DO
					GENERATION_STATUS resultDTC = patternGeneration((**it), true);
					if (resultDTC == TEST_FOUND)
					{
						clearAllFaultEffectBySimulation();
						storeCurrentGateValue();
						assignPatternPiValue(pcoll->pats_.back());
					}
					else
					{
						for (int i = 0; i < cir_->tgate_; ++i)
						{
							Gate &g = cir_->gates_[i];
							g.v_ = g.preV_;
						}
					}
				}
				else
				{
					setValueAndRunImp((*pGateForAtivation), pGateForAtivation->preV_);
				}
			}
		}

		clearAllFaultEffectBySimulation();
		storeCurrentGateValue();
		assignPatternPiValue(pcoll->pats_.back());

		if (pcoll->XFill_ == PatternProcessor::ON)
		{
			/*
			 * Randomly fill the pats_.back().
			 * Please note that the v_, gh_, gl_, fh_ and fl_ do not be changed.
			 * */
			randomFill(pcoll->pats_.back());
		}

		/*
		 * This function will assign pi/ppi stored in pats_.back() to
		 * the gh_ and gl_ in each gate, and then it will run fault
		 * simulation to drop fault.
		 * */
		sim_->pfFaultSim(pcoll->pats_.back(), faultListToGen);
		/*
		 * After sim_->pfFaultSim(pcoll->pats_.back(),faultListToGen) , the pi/ppi
		 * values have been passed to gh_ and gl_ of each gate.  Therefore, we can
		 * directly use "assignPatternPoValue" to perform goodSim to get the PoValue.
		 * */
		assignPatternPoValue(pcoll->pats_.back());
	}
	else if (result == UNTESTABLE)
	{
		faultListToGen.front()->faultState_ = Fault::AU;
		faultListToGen.pop_front();
		untest++;
	}
	else
	{
		faultListToGen.front()->faultState_ = Fault::AB;
		faultListToGen.push_back(faultListToGen.front());
		faultListToGen.pop_front();
	}
}

// **************************************************************************
// Function   [ Atpg::getWireForActivation ]
// Commentor  [ CAL ]
// Synopsis   [ usage:
//                return the gate need for activation of a fault
//              in:    void
//              out:   void
//            ]
// Date       [ started 2020/07/07    last modified 2020/07/07 ]
// **************************************************************************
Gate *Atpg::getWireForActivation(Fault &fault)
{
	bool isOutputFault = (fault.faultyLine_ == 0);
	Gate *pGateForAtivation;
	Gate *pFaultyGate = &cir_->gates_[fault.gateID_];
	if (!isOutputFault)
	{
		pGateForAtivation = &cir_->gates_[pFaultyGate->fis_[fault.faultyLine_ - 1]];
	}
	else
	{
		pGateForAtivation = pFaultyGate;
	}

	return pGateForAtivation;
}

// **************************************************************************
// Function   [ Atpg::reverseFaultSimulation ]
// Commentor  [ CAL ]
// Synopsis   [ usage:
//                Perform reverse fault simulation
//              in:    void
//              out:   void
//            ]
// Date       [ started 2020/07/08    last modified 2020/07/08 ]
// **************************************************************************
void Atpg::reverseFaultSimulation(PatternProcessor *pcoll, FaultList &originalFaultList)
{
	// set TD to UD
	for (auto it = originalFaultList.begin(); it != originalFaultList.end(); ++it)
	{
		(*it)->detection_ = 0;
		if ((*it)->faultState_ == Fault::DT)
		{
			(*it)->faultState_ = Fault::UD;
		}
	}

	PatternVec tmp = pcoll->pats_;
	pcoll->pats_.clear();

	// Perform reverse fault simulation
	size_t leftFaultCount = originalFaultList.size();
	for (auto rit = tmp.rbegin(); rit != tmp.rend(); ++rit)
	{
		sim_->pfFaultSim((*rit), originalFaultList);
		if (leftFaultCount > originalFaultList.size())
		{
			leftFaultCount = originalFaultList.size();
			pcoll->pats_.push_back((*rit));
		}
		else if (leftFaultCount < originalFaultList.size())
		{
			std::cout << "reverseFaultSimulation: unexpected behavior" << std::endl;
			std::cin.get();
		}
		else
		{
			delete (*rit);
		}
	}
}
