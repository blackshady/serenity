/*
 * Copyright (c) 2020, Matthew Olsson <mattco@serenityos.org>
 * Copyright (c) 2021-2023, Linus Groh <linusg@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Function.h>
#include <AK/TypeCasts.h>
#include <LibJS/Runtime/Completion.h>
#include <LibJS/Runtime/Error.h>
#include <LibJS/Runtime/GlobalObject.h>
#include <LibJS/Runtime/Object.h>
#include <LibJS/Runtime/PrimitiveString.h>
#include <LibJS/Runtime/SymbolObject.h>
#include <LibJS/Runtime/SymbolPrototype.h>
#include <LibJS/Runtime/Value.h>

namespace JS {

SymbolPrototype::SymbolPrototype(Realm& realm)
    : Object(ConstructWithPrototypeTag::Tag, *realm.intrinsics().object_prototype())
{
}

ThrowCompletionOr<void> SymbolPrototype::initialize(Realm& realm)
{
    auto& vm = this->vm();
    MUST_OR_THROW_OOM(Base::initialize(realm));
    u8 attr = Attribute::Writable | Attribute::Configurable;
    define_native_function(realm, vm.names.toString, to_string, 0, attr);
    define_native_function(realm, vm.names.valueOf, value_of, 0, attr);
    define_native_accessor(realm, vm.names.description, description_getter, {}, Attribute::Configurable);
    define_native_function(realm, *vm.well_known_symbol_to_primitive(), symbol_to_primitive, 1, Attribute::Configurable);

    // 20.4.3.6 Symbol.prototype [ @@toStringTag ], https://tc39.es/ecma262/#sec-symbol.prototype-@@tostringtag
    define_direct_property(*vm.well_known_symbol_to_string_tag(), MUST_OR_THROW_OOM(PrimitiveString::create(vm, "Symbol"sv)), Attribute::Configurable);

    return {};
}

// thisSymbolValue ( value ), https://tc39.es/ecma262/#thissymbolvalue
static ThrowCompletionOr<Symbol*> this_symbol_value(VM& vm, Value value)
{
    if (value.is_symbol())
        return &value.as_symbol();
    if (value.is_object() && is<SymbolObject>(value.as_object()))
        return &static_cast<SymbolObject&>(value.as_object()).primitive_symbol();
    return vm.throw_completion<TypeError>(ErrorType::NotAnObjectOfType, "Symbol");
}

// 20.4.3.2 get Symbol.prototype.description, https://tc39.es/ecma262/#sec-symbol.prototype.description
JS_DEFINE_NATIVE_FUNCTION(SymbolPrototype::description_getter)
{
    auto* symbol = TRY(this_symbol_value(vm, vm.this_value()));
    auto& description = symbol->description();
    if (!description.has_value())
        return js_undefined();
    return PrimitiveString::create(vm, *description);
}

// 20.4.3.3 Symbol.prototype.toString ( ), https://tc39.es/ecma262/#sec-symbol.prototype.tostring
JS_DEFINE_NATIVE_FUNCTION(SymbolPrototype::to_string)
{
    auto* symbol = TRY(this_symbol_value(vm, vm.this_value()));
    return PrimitiveString::create(vm, TRY_OR_THROW_OOM(vm, symbol->descriptive_string()));
}

// 20.4.3.4 Symbol.prototype.valueOf ( ), https://tc39.es/ecma262/#sec-symbol.prototype.valueof
JS_DEFINE_NATIVE_FUNCTION(SymbolPrototype::value_of)
{
    return TRY(this_symbol_value(vm, vm.this_value()));
}

// 20.4.3.5 Symbol.prototype [ @@toPrimitive ] ( hint ), https://tc39.es/ecma262/#sec-symbol.prototype-@@toprimitive
JS_DEFINE_NATIVE_FUNCTION(SymbolPrototype::symbol_to_primitive)
{
    // The hint argument is ignored.
    return TRY(this_symbol_value(vm, vm.this_value()));
}

}
