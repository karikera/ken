#pragma once

namespace v8
{
	class Data;
	class Private;
	class Value;
	class External;
	class Object;
	class ObjectTemplate;
	class Context;
	class Function;
	class FunctionTemplate;
	class Script;
	class TryCatch;
	class Array;

}

namespace kr
{
	template <typename T> class V8Handle;
	typedef V8Handle<v8::Data> V8Data;
	typedef V8Handle<v8::Private> V8Private;
	typedef V8Handle<v8::Value> V8Value;
	typedef V8Handle<v8::Object> V8Object;
	typedef V8Handle<v8::Array> V8Array;
	typedef V8Handle<v8::Function> V8Function;
	typedef V8Handle<v8::FunctionTemplate> V8Class;
}
