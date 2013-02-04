namespace NLP {
  namespace CRF {
    template <typename Node>
    class NodePool : public Util::Pool {
      private:
        Node *_free;

      public:
        NodePool(void) : Pool(512 * 1024), _free(0) { }

        void *alloc(size_t size) {
          if (_free) {
            Node *top = _free;
            _free = const_cast<Node *>(top->prev);
            return top;
          }
          return Util::Pool::alloc(size);
        }

        void free(Node *) { }

        void clear(void) {
          _free = NULL;
          Util::Pool::clear();
        }
    };
  }
}
