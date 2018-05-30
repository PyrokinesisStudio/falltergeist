/*
 * Copyright 2012-2014 Falltergeist Developers.
 *
 * This file is part of Falltergeist.
 *
 * Falltergeist is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Falltergeist is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Falltergeist.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FALLTERGEIST_VM_IFALLOUTCONTEXT_H
#define FALLTERGEIST_VM_IFALLOUTCONTEXT_H

// C++ standard includes

// Falltergeist includes
#include "../VM/IContext.h"

// Third party includes

namespace Falltergeist {
    namespace VM {
        class IFalloutProcedure;
        class IFalloutStack;

        class IFalloutContext : public virtual IContext {
        public:
            virtual ~IFalloutContext() = default;
            virtual std::shared_ptr<IFalloutStack> dataStack() = 0;
            virtual std::shared_ptr<IFalloutStack> returnStack() = 0;
            virtual void setProgramCounter(unsigned counter) = 0;
            virtual unsigned programCounter() const = 0;
            virtual std::shared_ptr<IFalloutProcedure> procedure(const std::string& name) const = 0;
            virtual std::shared_ptr<IFalloutProcedure> procedure(int index) const = 0;
        };
    }
}

#endif //FALLTERGEIST_VM_IFALLOUTCONTEXT_H