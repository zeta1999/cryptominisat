/***********************************************************************
CryptoMiniSat -- Copyright (c) 2009 Mate Soos

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
************************************************************************/

#ifndef CLAUSEALLOCATOR_H
#define CLAUSEALLOCATOR_H

#ifdef _MSC_VER
#include <msvc/stdint.h>
#else
#include <stdint.h>
#endif //_MSC_VER

#include <stdlib.h>
#include "Vec.h"
#include <map>
#include <vector>
using std::map;
using std::vector;

#include "ClauseOffset.h"
#include "Watched.h"

#ifdef USE_BOOST
#include <boost/pool/pool.hpp>
#endif //USE_BOOST

#define NUM_BITS_OUTER_OFFSET 4


class Clause;
class XorClause;
class Solver;


/**
@brief Allocates memory for (xor) clauses

This class allocates memory in large chunks, then distributes it to clauses when
needed. When instructed, it consolidates the unused space (i.e. clauses free()-ed).
Essentially, it is a stack-like allocator for clauses. It is useful to have
this, because this way, we can address clauses according to their number,
which is 32-bit, instead of their address, which might be 64-bit

2-long clauses are specially treated. If BOOST is enabled, a special memory
allocator is used for 2-long clauses. If BOOST is not available, then regular
memory allocation is used (i.e. not stack-based). This is because for 2-long
clauses, a 64-bit pointer doesn't cause problems (while for other clauses, it
does)
*/
class ClauseAllocator {
    public:
        ClauseAllocator();
        ~ClauseAllocator();
        
        template<class T>
        Clause* Clause_new(const T& ps, const uint32_t group, const bool learnt = false);
        template<class T>
        XorClause* XorClause_new(const T& ps, const bool inverted, const uint32_t group);
        Clause* Clause_new(Clause& c);

        const ClauseOffset getOffset(const Clause* ptr) const;

        inline Clause* getPointer(const uint32_t offset)
        {
            return (Clause*)(dataStarts[offset&((1 << NUM_BITS_OUTER_OFFSET) - 1)]
                            +(offset >> NUM_BITS_OUTER_OFFSET));
        }

        void clauseFree(Clause* c);

        void consolidate(Solver* solver);

    private:
        uint32_t getOuterOffset(const Clause* c) const;
        uint32_t getInterOffset(const Clause* c, const uint32_t outerOffset) const;
        const ClauseOffset combineOuterInterOffsets(const uint32_t outerOffset, const uint32_t interOffset) const;
        const bool insideMemoryRange(const Clause* c) const;

        void updateAllOffsetsAndPointers(Solver* solver);
        template<class T>
        void updatePointers(vec<T*>& toUpdate);
        void updatePointers(vector<Clause*>& toUpdate);
        void updatePointers(vector<XorClause*>& toUpdate);
        void updatePointers(vector<std::pair<Clause*, uint32_t> >& toUpdate);

        void updateOffsets(vec<vec<Watched> >& watches);
        
        vec<uint32_t*> dataStarts;
        vec<size_t> sizes;
        vec<vec<uint32_t> > origClauseSizes;
        vec<size_t> maxSizes;
        vec<size_t> currentlyUsedSizes;
        vec<uint32_t> origSizes;

        #ifdef USE_BOOST
        boost::pool<> clausePoolBin;
        #endif //USE_BOOST

        void* allocEnough(const uint32_t size);
};

#endif //CLAUSEALLOCATOR_H
