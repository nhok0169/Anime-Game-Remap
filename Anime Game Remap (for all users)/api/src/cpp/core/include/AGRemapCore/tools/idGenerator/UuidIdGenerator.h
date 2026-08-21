#ifndef AGRemapCore_UuidIdGenerator_H
#define AGRemapCore_UuidIdGenerator_H

#include <random>
#include <string>

#include "AGRemapCore/tools/idGenerator/BaseIdGenerator.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIdGenerator`

     A generator that generates ids as random `UUID`_ (version 4) strings, formatted the same way
     Python's ``str(uuid.uuid4())`` is (36 characters, lowercase hex, hyphenated 8-4-4-4-12) --
     used as the default state/production-bookkeeping id generator for
     :cpp:class:`BaseSLR1Parser`\\<std::string\\> :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Randomness comes from ``std::mt19937_64`` seeded off ``std::random_device``, not from the
        same source Python's ``uuid.uuid4()`` uses (``os.urandom``) -- irrelevant here since nothing
        needs the two to produce related/reproducible values, only for each generator to produce
        practically-unique ids on its own
     @endrst
     */
    class UuidIdGenerator: public BaseIdGenerator<std::string> {
        public:

            /**
             * @brief Constructs a new generator
             */
            UuidIdGenerator();

            /**
             * @brief
             @rst
             No-op -- there is no persistent counter/sequence state to reset; every generated id is
             independently random
             @endrst
             */
            void reset() override;

            bool getId(std::string& result) override;

        private:
            std::mt19937_64 engine_;
    };
}

#endif
