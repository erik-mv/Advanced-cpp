template <typename UnusedType>
class function;

template <typename ReturnType, typename... ArgumentTypes>
class function<ReturnType(ArgumentTypes...)> {
 public:
  function() : mInvoker() {}
  template <typename FunctionT>
  function(FunctionT f) : mInvoker(new free_function_holder<FunctionT>(f)) {}

  ReturnType operator()(ArgumentTypes... args) {
    return mInvoker->invoke(args...);
  }

  function(const function& other) : mInvoker(other.mInvoker->clone()) {}

  function& operator=(const function& other) {
    mInvoker = other.mInvoker->clone();
  }

  template <typename FunctionType, typename ClassType>
  function(FunctionType ClassType::*f)
      : mInvoker(
            new member_function_holder<FunctionType, ArgumentTypes...>(f)) {}

 private:
  class function_holder_base {
   public:
    function_holder_base() {}
    virtual ~function_holder_base() {}

    virtual ReturnType invoke(ArgumentTypes... args) = 0;
    virtual std::unique_ptr<function_holder_base> clone() = 0;

   private:
    function_holder_base(const function_holder_base&);
    void operator=(const function_holder_base&);
  };
  typedef std::unique_ptr<function_holder_base> invoker_t;

  template <typename FunctionT>
  class free_function_holder : public function_holder_base {
   public:
    free_function_holder(FunctionT func)
        : function_holder_base(), mFunction(func) {}

    virtual ReturnType invoke(ArgumentTypes... args) {
      return mFunction(args...);
    }

    typedef free_function_holder<FunctionT> self_type;
    virtual invoker_t clone() { return invoker_t(new self_type(mFunction)); }
    FunctionT mFunction;
  };
  invoker_t mInvoker;

  template <typename FunctionType, typename ClassType,
            typename... RestArgumentTypes>
  class member_function_holder : public function_holder_base {
   public:
    typedef FunctionType ClassType::*member_function_signature_t;
    member_function_holder(member_function_signature_t f) : mFunction(f) {}

    virtual ReturnType invoke(ClassType obj, RestArgumentTypes... restArgs) {
      return (obj.*mFunction)(restArgs...);
    }

    virtual invoker_t clone() {
      return invoker_t(new member_function_holder(mFunction));
    }

   private:
    member_function_signature_t mFunction;
  };
};