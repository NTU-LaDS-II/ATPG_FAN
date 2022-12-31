// **************************************************************************
// File       [ gate.h ]
// Author     [ littleshamoo ]
// Synopsis   [ ]
// Date       [ 2011/07/05 created ]
// **************************************************************************

#ifndef _CORE_GATE_H_
#define _CORE_GATE_H_

#include <cstring>
#include "interface/cell.h"
#include "logic.h"

namespace CoreNs
{
	class Gate
	{
	public:
		enum GateType
		{
			NA = 0,
			PI,
			PO,
			PPI,
			PPO,
			PPI_IN,
			PPO_IN,
			INV,
			BUF,
			AND2,
			AND3,
			AND4,
			NAND2,
			NAND3,
			NAND4,
			OR2,
			OR3,
			OR4,
			NOR2,
			NOR3,
			NOR4,
			XOR2,
			XOR3,
			XNOR2,
			XNOR3,
			MUX,
			TIE0,
			TIE1,
			TIEX,
			TIEZ
		};
		inline Gate();
		inline Gate(int gateId, int cellId, int primitiveId, int numLevel, GateType gateType, int numFO);

		// basic info
		int gateId_;				// position in circuit gate array
		int cellId_;				// original cell id in the netlist
		int primitiveId_;		// original primitive id in the library cell
		int numLevel_;			// level number after levelization
		int frame_;					// time frame of the gate, for 2-pattern test
		GateType gateType_; // type of the gate

		// connection
		int numFI_;											// number of fanin
		int numFO_;											// number of fanout
		std::vector<int> faninVector_;	// fanin array
		std::vector<int> fanoutVector_; // fanout array

		// values
		Value atpgVal_;					 // single value for ATPG
		ParaValue goodSimLow_;	 // good low
		ParaValue goodSimHigh_;	 // good high
		ParaValue faultSimLow_;	 // faulty low
		ParaValue faultSimHigh_; // faulty high

		// constraint, user can tie the gate to certain value
		bool hasConstraint_;
		ParaValue constraint_;

		// SCOAP testability
		int cc0_;
		int cc1_;
		int co_;

		int depthFromPo_;			 // depth from po, this is for fault effect propagation
		int minLevelOfFanins_; // the minimum level of the fanin gates, this is to justify the headline cone, (in atpg.cpp)

		Value isUnary() const;
		Value isInverse() const;
		Value getInputNonCtrlValue() const;
		Value getInputCtrlValue() const;
		Value getOutputCtrlValue() const;
		Value prevAtpgValStored_; // Added by Shi-Tang Liu
	};

	inline Gate::Gate()
	{
		gateId_ = -1;
		cellId_ = -1;
		primitiveId_ = 0;
		numLevel_ = -1;
		frame_ = 0;
		gateType_ = NA;
		numFI_ = 0;
		numFO_ = 0;
		atpgVal_ = X;
		goodSimLow_ = PARA_L;
		goodSimHigh_ = PARA_L;
		faultSimLow_ = PARA_L;
		faultSimHigh_ = PARA_L;
		hasConstraint_ = false;
		constraint_ = PARA_L;
		cc0_ = 0;
		cc1_ = 0;
		co_ = 0;
		depthFromPo_ = -1;
		minLevelOfFanins_ = -1;
		prevAtpgValStored_ = X; // Added by Shi-Tang Liu
	}

	inline Gate::Gate(int gateId, int cellId, int primitiveId, int numLevel, GateType gateType, int numFO)
			: gateId_(gateId),
				cellId_(cellId),
				primitiveId_(primitiveId),
				numLevel_(numLevel),
				gateType_(gateType),
				numFO_(numFO)
	{
		frame_ = 0;
		numFI_ = 0;
		fanoutVector_ = std::vector<int>(numFO);
		atpgVal_ = X;
		goodSimLow_ = PARA_L;
		goodSimHigh_ = PARA_L;
		faultSimLow_ = PARA_L;
		faultSimHigh_ = PARA_L;
		hasConstraint_ = false;
		constraint_ = PARA_L;
		cc0_ = 0;
		cc1_ = 0;
		co_ = 0;
		depthFromPo_ = -1;
		minLevelOfFanins_ = -1;
		prevAtpgValStored_ = X; // Added by Shi-Tang Liu
	}

	inline Value Gate::isUnary() const
	{
		return numFI_ == 1 ? H : L;
	}

	inline Value Gate::isInverse() const
	{
		switch (gateType_)
		{
			case INV:
			case NAND2:
			case NAND3:
			case NAND4:
			case NOR2:
			case NOR3:
			case NOR4:
			case XNOR2:
			case XNOR3:
				return H;
			default:
				return L;
		}
	}

	inline Value Gate::getInputNonCtrlValue() const
	{
		return (isInverse() == getOutputCtrlValue()) ? L : H;
	}

	inline Value Gate::getInputCtrlValue() const
	{
		return (getInputNonCtrlValue() == H) ? L : H;
	}

	inline Value Gate::getOutputCtrlValue() const
	{
		switch (gateType_)
		{
			case OR2:
			case OR3:
			case OR4:
			case NAND2:
			case NAND3:
			case NAND4:
				return L;
			case XOR2:
			case XOR3:
			case XNOR2:
			case XNOR3:
				return X;
			default:
				return H;
		}
	}

};

#endif