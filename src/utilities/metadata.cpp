#include "lexer/lexer.h"
// #include "parser/module.h"

#include "ast/nodes.h"
#include "builtin/operators.h"
#include "parser/parser.h"
#include "sema/sema.h"
#include "ast/values/native.h"
#include "ast/values/object.h"

#include "allocator.h"
#include "metadata_1.h"
#include "metadata.h"
#include "ast/values/exception.h"

namespace lython {


#if (defined __clang__)

#elif (defined __GNUC__)
// Shared Pointer & Hash tables do not actually allocate T
// but a struct that includes T and other data
// So we rename that mangled struct name by that type T it manages
// Those details are implementation specific to GCC
template <typename T>
using SharedPtrInternal = std::_Sp_counted_ptr_inplace<T, lython::Allocator<T, device::CPU>, std::__default_lock_policy>;

template <typename T, bool cache>
using HashNodeInternal = std::__detail::_Hash_node<T, cache>;

#else
//#elif !BUILD_WEBASSEMBLY && !__clang__
template <typename T>
using SharedPtrInternal = std::shared_ptr<T>;

template <typename T, bool cache>
using HashNodeInternal = std::_List_node<T, void* __ptr64>;

template <typename T, bool cache>
using ListIterator = std::_List_unchecked_iterator<std::_List_val<std::_List_simple_types<T>>>;
#endif

template <typename T>
using UniquePtrInternal = std::unique_ptr<T>;

namespace meta {
int _register_type_once(int tid, const char* str) {
    if (!is_type_registry_available())
        return 0;

    auto& db = TypeRegistry::instance().id_to_meta;
    auto result = db.find(tid);

    if (result == db.end() || result->second.name == "") {
        db[tid].name = str;
        db[tid].type_id = tid;
    }
    return tid;
}
}


void _metadata_init_names_windows();
void _metadata_init_names_unix();
void _metadata_init_names_gcc();
void _metadata_init_names_clang();
void _metadata_init_names_js();

bool _metadata_init_names() {
    _metadata_init_names_windows();
    _metadata_init_names_unix();
    _metadata_init_names_js();
    _metadata_init_names_gcc();
    _metadata_init_names_clang();

    meta::override_typename<UniquePtr<lython::Module>>("UniquePtr[Module]");


    meta::override_typename<char>("char");
    meta::override_typename<int>("int");
    meta::override_typename<lython::NativeObject*>("NativeObject*");
    // meta::override_typename<lython::NativePointer<>*>("NativePointer*");
    meta::override_typename<lython::lyException*>("Exception*");
    meta::override_typename<lython::Object>("Object");
    meta::override_typename<lython::lyException>("Exception");
    meta::override_typename<lython::Node>("Node");
    meta::override_typename<lython::StackTrace>("StackTrace");

    meta::override_typename<lython::String>("String");
    meta::override_typename<lython::StringRef>("StringRef");
    meta::override_typename<lython::StringDatabase::StringEntry>("StringDatabase::StringEntry");

    meta::override_typename<lython::Node*>("Node*");
    meta::override_typename<lython::GCObject*>("GCObject*");
    meta::override_typename<lython::ExprNode*>("ExprNode*");
    meta::override_typename<lython::StmtNode*>("StmtNode*");
    meta::override_typename<lython::ConstantValue>("ConstantValue");
    meta::override_typename<lython::BinOp::NativeBinaryOp>("NativeBinaryOperation");

    meta::override_typename<UniquePtrInternal<lython::SemaException>>("SemaException");
    meta::override_typename<UniquePtrInternal<lython::ParsingException>>("ParsingException");

    meta::override_typename<lython::Comprehension>("Comprehension");
    meta::override_typename<lython::Alias>("Alias");
    meta::override_typename<lython::WithItem>("WithItem");
    meta::override_typename<lython::ExceptHandler>("ExceptHandler");
    meta::override_typename<lython::Arg>("Arg");
    meta::override_typename<lython::CmpOperator>("CmpOperator");
    meta::override_typename<lython::Keyword>("Keyword");
    meta::override_typename<lython::MatchCase>("MatchCase");
    meta::override_typename<lython::Pattern*>("Pattern*");
    meta::override_typename<lython::BindingEntry>("BindingEntry");
    meta::override_typename<Array<StmtNode*>>("Array<StmtNode*>");
    meta::override_typename<ExprContext>("ExprContext");
    meta::override_typename<ClassDef::Attr>("ClassDef::Attr");
    meta::override_typename<ParsingContext>("ParsingContext");
    meta::override_typename<SemaContext>("SemaContext");
    meta::override_typename<Decorator>("Decorator");
    meta::override_typename<Token>("Token");
    meta::override_typename<ParsingError>("ParsingError");

#define REGISTER_TYPE(type)                       \
    meta::override_typename<lython::type>(#type); \
    meta::override_typename<lython::type*>(#type "*");

#define X(name, _)
#define SECTION(name)
#define EXPR(name, _)  REGISTER_TYPE(name)
#define STMT(name, _)  REGISTER_TYPE(name)
#define MOD(name, _)   REGISTER_TYPE(name)
#define MATCH(name, _) REGISTER_TYPE(name)

    NODEKIND_ENUM(X, SECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

// __linux__ || (!BUILD_WEBASSEMBLY && !__clang__)
#if (defined __clang__)

#elif (defined __GNUC__) 
    meta::override_typename<
        HashNodeInternal<std::pair<const StringRef, lython::ClassDef::Attr>, false>>(
        "Pair[Ref, Classdef::Attr]");

    meta::override_typename<
        HashNodeInternal<std::pair<const StringRef, lython::ImportLib::ImportedLib>, false>>(
        "Pair[Ref, ImportLib::ImportedLib]");

    meta::override_typename<HashNodeInternal<std::pair<const String, lython::OpConfig>, false>>(
        "Pair[String, OpConfig]");

    meta::override_typename<HashNodeInternal<std::pair<const String, lython::TokenType>, false>>(
        "Pair[String, TokenType]");

    meta::override_typename<HashNodeInternal<std::pair<const String, TokenType>, true>>(
        "Pair[String, TokenType]");

    meta::override_typename<HashNodeInternal<std::pair<const String, OpConfig>, true>>(
        "Pair[String, OpConfig]");

    meta::override_typename<HashNodeInternal<std::pair<const StringRef, bool>, true>>(
        "Pair[String, bool]");

    meta::override_typename<HashNodeInternal<std::pair<const StringRef, lython::ExprNode*>, true>>(
        "Pair[String, ExprNode*]");

    meta::override_typename<
        HashNodeInternal<std::pair<const StringRef, lython::BinOp::NativeBinaryOp>, true>>(
        "Pair[String, NativeBinaryOp]");

    meta::override_typename<
        HashNodeInternal<std::pair<const StringRef, lython::UnaryOp::NativeUnaryOp>, true>>(
        "Pair[String, NativeUnaryOp]");

    // String Database
    meta::override_typename<Array<StringDatabase::StringEntry>*>("Array[StringEntry]*");

    // StringDatabase
    meta::override_typename<HashNodeInternal<std::pair<const StringView, std::size_t>, true>>(
        "Pair[StringView, size_t]");

    meta::override_typename<HashNodeInternal<std::pair<const StringRef, lython::ExprNode*>, false>>(
        "Pair[StringRef, ExprNode*]");

    meta::override_typename<
        HashNodeInternal<std::pair<const StringRef, lython::BinOp::NativeBinaryOp>, false>>(
        "Pair[StringRef, NativeBinaryOp]");

    meta::override_typename<
        HashNodeInternal<std::pair<const StringRef, lython::UnaryOp::NativeUnaryOp>, false>>(
        "Pair[StringRef, NativeUnaryOp]");

    meta::override_typename<HashNodeInternal<std::pair<const StringRef, bool>, false>>(
        "Pair[StringRef, bool]");

    // module
    meta::override_typename<HashNodeInternal<std::pair<const String, int>, true>>(
        "Pair[String, Index]");

    // module precedence_table
    meta::override_typename<HashNodeInternal<std::pair<const String, std::tuple<int, bool>>, true>>(
        "Pair[String, Tuple[int, bool]]");

    // Keyword to string
    meta::override_typename<HashNodeInternal<std::pair<const int, String>, false>>(
        "Pair[int, String]");

    // Set Keyword
    meta::override_typename<HashNodeInternal<char, false>>("Set[char]");
#endif

#define INIT_METADATA(name, typname) meta::type_name<name>();

    TYPES_METADATA(INIT_METADATA)

    return true;
}

void _metadata_init_names_gcc() {}
void _metadata_init_names_clang() {}

void _metadata_init_names_windows() {
#if (!defined __linux__) && (!BUILD_WEBASSEMBLY && !__clang__)
    meta::override_typename<
        ListIterator<std::pair<const StringRef, lython::ClassDef::Attr>, false>>(
        "Iterator[Pair[Ref, Classdef::Attr]]");

    meta::override_typename<ListIterator<std::pair<const int, String>, false>>(
        "Iterator[Pair[int, String]]");

    meta::override_typename<ListIterator<std::pair<const std::string_view, std::size_t>, false>>(
        "Iterator[Pair[StringView, size_t]]");

    meta::override_typename<ListIterator<std::pair<const String, OpConfig>, false>>(
        "Iterator[Pair[String, OpConfig]]");

    meta::override_typename<ListIterator<std::pair<const String, TokenType>, false>>(
        "Iterator[Pair[String, TokenType]]");

    meta::override_typename<
        ListIterator<std::pair<const StringRef, lython::BinOp::NativeBinaryOp>, false>>(
        "Iterator[Pair[StringRef, NativeBinaryOp]]");

    meta::override_typename<
        ListIterator<std::pair<const StringRef, lython::UnaryOp::NativeUnaryOp>, false>>(
        "Iterator[Pair[StringRef, NativeUnaryOp]]");

    meta::override_typename<ListIterator<std::pair<const StringRef, lython::ExprNode*>, false>>(
        "Iterator[Pair[StringRef, ExprNode*]]");

    meta::override_typename<ListIterator<std::pair<const StringRef, bool>, false>>(
        "Iterator[Pair[StringRef, bool]]");

    meta::override_typename<
        std::_List_node<Array<StringDatabase::StringEntry>,
                        typename std::allocator_traits<
                            std::allocator<Array<StringDatabase::StringEntry>>>::void_pointer>>(
        "ListNode[Array[StringEntry]]");
#endif
}
void _metadata_init_names_unix() {

#if (defined __clang__)

#elif (defined __linux__)
    meta::override_typename<std::_List_node<Array<StringDatabase::StringEntry>>>(
        "ListNode[StringEntry]");
#endif
}
void _metadata_init_names_js() {}

void track_static() {
    // Record the allocation count on startup
    // so we can try to ignore static variables
    // this will only work if `metadata_init_names` is called
    // after the static variables got initialized
    for (auto& s: meta::TypeRegistry::instance().id_to_meta) {
        s.second.stat.startup_count = 0;
        // s.allocated - s.deallocated;
    }
}

void metadata_init_names() { static bool _ = _metadata_init_names(); }

void register_globals() {
    {
        metadata_init_names();

        // Static globals
        {
            StringDatabase::instance();
            default_precedence();
            keywords();
            keyword_as_string();
            native_binary_operators();
            native_bool_operators();
            native_unary_operators();
            native_cmp_operators();
            operator_magic_name(BinaryOperator::Add);
            operator_magic_name(BoolOperator::And);
            operator_magic_name(UnaryOperator::Invert);
            operator_magic_name(CmpOperator::Eq);

            None();
            True();
            False();

            strip_defaults();
        }

        // --
        track_static();
    }
}

namespace meta {
TypeRegistry& TypeRegistry::instance() {
    static TypeRegistry obj;
    return obj;
}

TypeRegistry::TypeRegistry() { is_type_registry_available() = true; }

TypeRegistry::~TypeRegistry() {
    if (print_stats) {
        show_alloc_stats();
    }

    is_type_registry_available() = false;
}

bool& is_type_registry_available() {
    static bool avail = false;
    return avail;
}


ClassMetadata& classmeta(int _typeid) {
    return TypeRegistry::instance().id_to_meta[_typeid];
}

Member const& nomember() {
    static Member const m("", -1, -1);
    return m;
}

Member const& member(int _typeid, int id) {
    ClassMetadata& registry = classmeta(_typeid);
    if (id >= registry.members.size()) {
        assert(0, "Member should exist");
        return nomember();
    }
    return registry.members[id];
}

Member const& member(int _typeid, std::string const& name) {
    ClassMetadata& registry = classmeta(_typeid);

    for (Member& member: registry.members) {
        if (member.name == name) {
            return member;
        }
    }

    return nomember();
}

void print(std::ostream& ss, int typeid_, std::int8_t const* data) {
    ClassMetadata const& registry = classmeta(typeid_);

    if (registry.printer) {
        registry.printer(ss, data);
    }
}

}  // namespace meta

}  // namespace lython
