#include "mrb_libzmq.h"

static mrb_value
mrb_zmq_bind(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *endpoint;
  mrb_get_args(mrb, "dz", &socket, &mrb_zmq_socket_type, &endpoint);

  int rc = zmq_bind(socket, endpoint);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_bind");
  }

  return self;
}

static mrb_value
mrb_zmq_close(mrb_state *mrb, mrb_value self)
{
  mrb_value socket_val;
  mrb_get_args(mrb, "o", &socket_val);

  if (mrb_type(socket_val) == MRB_TT_DATA && DATA_TYPE(socket_val) == &mrb_zmq_socket_type) {
    int rc = zmq_close(DATA_PTR(socket_val));
    if (unlikely(rc == -1)) {
      mrb_zmq_handle_error(mrb, "zmq_close");
    }
    mrb_data_init(socket_val, NULL, NULL);
  }

  return mrb_nil_value();
}

static mrb_value
mrb_zmq_close_mark(mrb_state *mrb, mrb_value self)
{
  mrb_value socket_val;
  mrb_get_args(mrb, "o", &socket_val);

  if (mrb_type(socket_val) == MRB_TT_DATA && DATA_TYPE(socket_val) == &mrb_zmq_socket_type) {
    int disable = 0;
    zmq_setsockopt(DATA_PTR(socket_val), ZMQ_LINGER, &disable, sizeof(disable));
    int rc = zmq_close(DATA_PTR(socket_val));
    if (unlikely(rc == -1)) {
      mrb_zmq_handle_error(mrb, "zmq_close");
    }
    mrb_data_init(socket_val, NULL, NULL);
  }

  return mrb_nil_value();
}

static mrb_value
mrb_zmq_connect(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *endpoint;
  mrb_get_args(mrb, "dz", &socket, &mrb_zmq_socket_type, &endpoint);

  int rc = zmq_connect(socket, endpoint);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_connect");
  }

  return self;
}

static mrb_value
mrb_zmq_ctx_get(mrb_state *mrb, mrb_value self)
{
  mrb_int option_name;
  mrb_get_args(mrb, "i", &option_name);

  int rc = zmq_ctx_get(MRB_LIBZMQ_CONTEXT(mrb), (int) option_name);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_ctx_get");
  }

  return mrb_int_value(mrb, rc);
}

static mrb_value
mrb_zmq_ctx_set(mrb_state *mrb, mrb_value self)
{
  mrb_int option_name, option_value;
  mrb_get_args(mrb, "ii", &option_name, &option_value);

  int rc = zmq_ctx_set(MRB_LIBZMQ_CONTEXT(mrb), (int) option_name, (int) option_value);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_ctx_set");
  }

  return self;
}

static mrb_value
mrb_zmq_curve_keypair(mrb_state *mrb, mrb_value self)
{
  mrb_value public_key = mrb_str_new(mrb, NULL, 40);
  mrb_value secret_key = mrb_str_new(mrb, NULL, 40);

  int rc = zmq_curve_keypair(RSTRING_PTR(public_key), RSTRING_PTR(secret_key));
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_curve_keypair");
  }

  mrb_value keypair = mrb_hash_new_capa(mrb, 2);
  mrb_hash_set(mrb, keypair, mrb_symbol_value(mrb_intern_lit(mrb, "public_key")), public_key);
  mrb_hash_set(mrb, keypair, mrb_symbol_value(mrb_intern_lit(mrb, "secret_key")), secret_key);

  return keypair;

}

static mrb_value
mrb_zmq_disconnect(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *endpoint;
  mrb_get_args(mrb, "dz", &socket, &mrb_zmq_socket_type, &endpoint);

  int rc = zmq_disconnect(socket, endpoint);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_disconnect");
  }

  return self;
}

static mrb_value
mrb_zmq_getsockopt(mrb_state *mrb, mrb_value self)
{
  void *socket;
  mrb_int option_name;
  mrb_value option_type;
  mrb_int string_return_len = 4096 - sizeof(mrb_value) - sizeof(struct RString);
  mrb_get_args(mrb, "diC|i", &socket, &mrb_zmq_socket_type, &option_name, &option_type, &string_return_len);
  assert(string_return_len >= 0);

  struct RClass* option_class = mrb_class_ptr(option_type);

  size_t option_len;
  int rc;

  if (option_name == ZMQ_FD) {
    SOCKET fd;
    option_len = sizeof(fd);
    rc = zmq_getsockopt(socket, option_name, &fd, &option_len);
    if (unlikely(rc == -1)) {
      mrb_zmq_handle_error(mrb, "zmq_getsockopt");
   }
    return mrb_int_value(mrb, fd);
  }
  else if (option_class == mrb->true_class||option_class == mrb->false_class) {
    int boolean;
    option_len = sizeof(boolean);
    rc = zmq_getsockopt(socket, option_name, &boolean, &option_len);
    if (unlikely(rc == -1)) {
      mrb_zmq_handle_error(mrb, "zmq_getsockopt");
    }
    return mrb_bool_value(boolean);
  }
  else if (option_class == mrb->integer_class) {
    int number;
    option_len = sizeof(number);
    rc = zmq_getsockopt(socket, option_name, &number, &option_len);
    if (unlikely(rc == -1)) {
      mrb_zmq_handle_error(mrb, "zmq_getsockopt");
    }
    return mrb_int_value(mrb, number);
  }
  else if (option_class == mrb->float_class) {
    int64_t number;
    option_len = sizeof(number);
    rc = zmq_getsockopt(socket, option_name, &number, &option_len);
    if (unlikely(rc == -1)) {
      mrb_zmq_handle_error(mrb, "zmq_getsockopt");
    }
#if (MRB_INT_BIT >= 64)
    return mrb_int_value(mrb, number);
#else
    return mrb_float_value(mrb, number);
#endif
  }
  else if (option_class == mrb->string_class) {
    mrb_value buf = mrb_str_new(mrb, NULL, string_return_len);
    option_len = RSTRING_CAPA(buf);
    rc = zmq_getsockopt(socket, option_name, RSTRING_PTR(buf), &option_len);
    if (unlikely(rc == -1)) {
      mrb_zmq_handle_error(mrb, "zmq_getsockopt");
    }

    return mrb_str_resize(mrb, buf, option_len);
  }
  else {
    mrb_raise(mrb, E_TYPE_ERROR, "Expected True-/FalseClass|Integer|Float|String");
  }

  return self;
}

static mrb_value
mrb_zmq_msg_new(mrb_state *mrb, mrb_value self)
{
  mrb_value data = mrb_nil_value();
  mrb_get_args(mrb, "|o", &data);

  zmq_msg_t *msg = (zmq_msg_t *) mrb_realloc(mrb, DATA_PTR(self), sizeof(*msg));
  memset(msg, 0, sizeof(*msg));
  mrb_data_init(self, msg, &mrb_zmq_msg_type);

  switch (mrb_type(data)) {
    case MRB_TT_STRING: {
      int rc = zmq_msg_init_size(msg, RSTRING_LEN(data));
      if (unlikely(rc == -1)) {
        mrb_zmq_handle_error(mrb, "zmq_msg_init_size");
      }
      memcpy(zmq_msg_data(msg), RSTRING_PTR(data), RSTRING_LEN(data));
    } break;
    case MRB_TT_FALSE: {
      zmq_msg_init(msg);
    } break;
    default: {
      mrb_raise(mrb, E_TYPE_ERROR, "(optionally) expected a String");
    }
  }

  return self;
}

static mrb_value
mrb_zmq_msg_copy(mrb_state *mrb, mrb_value copy)
{
  void* msg_src;
  mrb_get_args(mrb, "d", &msg_src, &mrb_zmq_msg_type);
  if (msg_src && msg_src == DATA_PTR(copy)) return copy;

  if (unlikely(!msg_src)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "uninitialized src msg");
  }

  zmq_msg_t *msg_copy = (zmq_msg_t *) mrb_realloc(mrb, DATA_PTR(copy), sizeof(*msg_copy));
  memset(msg_copy, 0, sizeof(*msg_copy));
  mrb_data_init(copy, msg_copy, &mrb_zmq_msg_type);
  zmq_msg_init(msg_copy);
  int rc = zmq_msg_copy(msg_copy, msg_src);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_msg_copy");
  }
  return copy;
}

static mrb_value
mrb_zmq_msg_gets(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *msg;
  char *property;
  mrb_get_args(mrb, "dz", &msg, &mrb_zmq_msg_type, &property);

  const char *prop = zmq_msg_gets(msg, property);
  if (prop) {
    return mrb_str_new_cstr(mrb, prop);
  } else {
    return mrb_nil_value();
  }
}

static mrb_value
mrb_zmq_msg_send(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *msg;
  void *socket;
  mrb_int flags;
  mrb_get_args(mrb, "ddi", &msg, &mrb_zmq_msg_type, &socket, &mrb_zmq_socket_type, &flags);
  mrb_assert_int_fit(mrb_int, flags, int, INT_MAX);

  int rc = zmq_msg_send(msg, socket, flags);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_msg_send");
  }

  return mrb_nil_value();
}

static mrb_value
mrb_zmq_msg_size(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *msg;
  mrb_get_args(mrb, "d", &msg, &mrb_zmq_msg_type);

  return mrb_int_value(mrb, zmq_msg_size(msg));
}

static mrb_value
mrb_zmq_proxy(mrb_state *mrb, mrb_value self)
{
  void *frontend, *backend, *capture = NULL;
  mrb_get_args(mrb, "dd|d!", &frontend, &mrb_zmq_socket_type, &backend, &mrb_zmq_socket_type, &capture, &mrb_zmq_socket_type);

  int rc = zmq_proxy(frontend, backend, capture);
  if (rc == -1) {
    mrb_zmq_handle_error(mrb, "zmq_proxy");
  }

  return self;
}

static mrb_value
mrb_zmq_proxy_steerable(mrb_state *mrb, mrb_value self)
{
  void *frontend, *backend, *control, *capture = NULL;
  mrb_get_args(mrb, "ddd|d!", &frontend, &mrb_zmq_socket_type, &backend, &mrb_zmq_socket_type, &control, &mrb_zmq_socket_type,
    &capture, &mrb_zmq_socket_type);

  int rc = zmq_proxy_steerable(frontend, backend, capture, control);
  if (rc == -1) {
    mrb_zmq_handle_error(mrb, "zmq_proxy");
  }

  return self;
}

static mrb_value
mrb_zmq_msg_to_str(mrb_state *mrb, mrb_value self)
{
  return mrb_str_new(mrb, (const char *) zmq_msg_data((zmq_msg_t *) DATA_PTR(self)), zmq_msg_size((zmq_msg_t *) DATA_PTR(self)));
}

static mrb_value
mrb_zmq_msg_eql(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *other;
  mrb_get_args(mrb, "d", &other, &mrb_zmq_msg_type);

  zmq_msg_t *msg = (zmq_msg_t *) DATA_PTR(self);

  if (msg == other) {
    return mrb_true_value();
  } else if (zmq_msg_size(msg) != zmq_msg_size(other)) {
    return mrb_false_value();
  } else {
    return mrb_bool_value(memcmp(zmq_msg_data(msg), zmq_msg_data(other), zmq_msg_size(msg)) == 0);
  }
}

static mrb_value
mrb_zmq_send(mrb_state *mrb, mrb_value self)
{
  void *socket;
  mrb_value message;
  mrb_int flags;
  mrb_get_args(mrb, "doi", &socket, &mrb_zmq_socket_type, &message, &flags);
  mrb_assert_int_fit(mrb_int, flags, int, INT_MAX);

  message = mrb_str_to_str(mrb, message);

  int rc = zmq_send(socket, RSTRING_PTR(message), RSTRING_LEN(message), flags);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_send");
  }

  return mrb_int_value(mrb, rc);
}

static mrb_value
mrb_zmq_setsockopt(mrb_state *mrb, mrb_value self)
{
  void *socket;
  mrb_int option_name;
  mrb_value option_value;
  mrb_get_args(mrb, "dio", &socket, &mrb_zmq_socket_type, &option_name, &option_value);
  mrb_assert_int_fit(mrb_int, option_name, int, INT_MAX);

  int rc = 0;

  switch(mrb_type(option_value)) {
    case MRB_TT_FALSE: {
      if (!mrb_integer(option_value)) {
        rc = zmq_setsockopt(socket, option_name, NULL, 0);
      } else {
        int boolean = 0;
        rc = zmq_setsockopt(socket, option_name, &boolean, sizeof(boolean));
      }
    } break;
    case MRB_TT_TRUE: {
      int boolean = 1;
      rc = zmq_setsockopt(socket, option_name, &boolean, sizeof(boolean));
    } break;
    case MRB_TT_INTEGER: {
      int number = (int) mrb_integer(option_value);
      rc = zmq_setsockopt(socket, option_name, &number, sizeof(number));
    } break;
    case MRB_TT_FLOAT: {
      int64_t number = (int64_t) mrb_float(option_value);
      rc = zmq_setsockopt(socket, option_name, &number, sizeof(number));
    } break;
    case MRB_TT_STRING: {
      rc = zmq_setsockopt(socket, option_name, RSTRING_PTR(option_value), RSTRING_LEN(option_value));
    } break;
    default: {
      mrb_raise(mrb, E_TYPE_ERROR, "expected nil|false|true|Integer|Float|String as value");
    }
  }

  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_setsockopt");
  }

  return self;
}

static mrb_value
mrb_zmq_unbind(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *endpoint;
  mrb_get_args(mrb, "dz", &socket, &mrb_zmq_socket_type, &endpoint);

  int rc = zmq_unbind(socket, endpoint);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_unbind");
  }

  return self;
}

#ifdef ZMQ_HAS_CAPABILITIES
static mrb_value
mrb_zmq_has(mrb_state *mrb, mrb_value self)
{
  char *capability;
  mrb_get_args(mrb, "z", &capability);

  return mrb_bool_value(zmq_has(capability));
}
#endif

#ifdef ZMQ_SERVER
static mrb_value
mrb_zmq_msg_routing_id(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *msg;
  mrb_get_args(mrb, "d", &msg, &mrb_zmq_msg_type);

  return mrb_int_value(mrb, zmq_msg_routing_id(msg));
}

static mrb_value
mrb_zmq_msg_set_routing_id(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *msg;
  mrb_int routing_id;
  mrb_get_args(mrb, "di", &msg, &mrb_zmq_msg_type, &routing_id);

  int rc = zmq_msg_set_routing_id(msg, (uint32_t) routing_id);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_msg_set_routing_id");
  }

  return self;
}
#endif // ZMQ_SERVER

#ifdef ZMQ_DISH
static mrb_value
mrb_zmq_join(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *group;
  mrb_get_args(mrb, "dz", &socket, &mrb_zmq_socket_type, &group);

  int rc = zmq_join(socket, group);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_join");
  }

  return self;
}

static mrb_value
mrb_zmq_leave(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *group;
  mrb_get_args(mrb, "dz", &socket, &mrb_zmq_socket_type, &group);

  int rc = zmq_leave(socket, group);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_leave");
  }

  return self;
}

static mrb_value
mrb_zmq_msg_group(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *msg;
  mrb_get_args(mrb, "d", &msg, &mrb_zmq_msg_type);

  return mrb_str_new_cstr(mrb, zmq_msg_group(msg));
}

static mrb_value
mrb_zmq_msg_set_group(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *msg;
  char *group;
  mrb_get_args(mrb, "dz", &msg, &mrb_zmq_msg_type, &group);

  int rc = zmq_msg_set_group(msg, group);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_msg_set_group");
  }

  return self;
}
#endif // ZMQ_DISH

static mrb_value
mrb_zmq_socket(mrb_state *mrb, mrb_value self)
{
  if (unlikely(DATA_PTR(self))) {
    mrb_free(mrb, DATA_PTR(self));
    mrb_data_init(self, NULL, NULL);
  }

  mrb_int type;
  mrb_get_args(mrb, "i", &type);
  mrb_assert_int_fit(mrb_int, type, int, INT_MAX);

  void *socket = zmq_socket(MRB_LIBZMQ_CONTEXT(mrb), (int) type);
  if (likely(socket)) {
    mrb_data_init(self, socket, &mrb_zmq_socket_type);
  } else {
    mrb_zmq_handle_error(mrb, "zmq_socket");
  }

  return self;
}

static mrb_value
mrb_zmq_socket_monitor(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *addr;
  mrb_int events;
  mrb_get_args(mrb, "dzi", &socket, &mrb_zmq_socket_type, &addr, &events);
  mrb_assert_int_fit(mrb_int, events, int, INT_MAX);

  int rc = zmq_socket_monitor(socket, addr, (int) events);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_socket_monitor");
  }

  return self;
}

static mrb_value
mrb_zmq_socket_recv(mrb_state *mrb, mrb_value self)
{
  mrb_int flags = 0;
  mrb_get_args(mrb, "|i", &flags);
  mrb_assert_int_fit(mrb_int, flags, int, INT_MAX);

  int more;
  mrb_value data = mrb_nil_value();
  struct RClass *zmq_msg_class = mrb_class_get_under(mrb, mrb_module_get(mrb, "ZMQ"), "Msg");

  do {
    mrb_value msg_val = mrb_obj_new(mrb, zmq_msg_class, 0, NULL);
    zmq_msg_t *msg = (zmq_msg_t *) DATA_PTR(msg_val);

    int rc = zmq_msg_recv (msg, DATA_PTR(self), (int) flags);
    if (unlikely(rc == -1)) {
      mrb_zmq_handle_error(mrb, "zmq_msg_recv");
    }
    more = zmq_msg_more(msg);
    if (more) {
      if (!mrb_array_p(data)) {
        data = mrb_ary_new_capa(mrb, 2); // We have at least two zmq messages at this point.
      }
      mrb_ary_push(mrb, data, msg_val);
    } else {
      if (mrb_array_p(data)) {
        mrb_ary_push(mrb, data, msg_val);
      } else {
        data = msg_val;
      }
    }
  } while (more);

  return data;
}

static mrb_value
mrb_zmq_z85_decode(mrb_state *mrb, mrb_value self)
{
  char *string;
  mrb_get_args(mrb, "z", &string);
  size_t string_len = strlen(string);

  if (unlikely(string_len % 5)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "string len must be divisible by 5");
  }

  mrb_value dest = mrb_str_new(mrb, NULL, 0.8 * string_len);

  uint8_t *rc = zmq_z85_decode((uint8_t *) RSTRING_PTR(dest), string);
  if (unlikely(!rc)) {
    mrb_zmq_handle_error(mrb, "zmq_z85_decode");
  }

  return dest;
}

static mrb_value
mrb_zmq_z85_encode(mrb_state *mrb, mrb_value self)
{
  char *data;
  mrb_int size;

  mrb_get_args(mrb, "s", &data, &size);

  if (unlikely(size % 4)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "data size must be divisible by 4");
  }

  mrb_value dest = mrb_str_new(mrb, NULL, mrb_integer(mrb_float_to_integer(mrb, mrb_num_mul(mrb, mrb_int_value(mrb, size), mrb_float_value(mrb, 1.25)))));

  char *rc = zmq_z85_encode(RSTRING_PTR(dest), (uint8_t *) data, size);
  if (unlikely(!rc)) {
    mrb_zmq_handle_error(mrb, "zmq_z85_encode");
  }

  return dest;
}

static void
mrb_zmq_thread_fn_cb(mrb_state *mrb, const mrb_value mrb_zmq_thread_data_)
{
  mrb_zmq_thread_data_t *mrb_zmq_thread_data = (mrb_zmq_thread_data_t *) mrb_cptr(mrb_zmq_thread_data_);

  struct RClass* zmq_mod = mrb_module_get(mrb, "ZMQ");
  mrb_value backend_pipe = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, mrb_class_get_under(mrb, zmq_mod, "Pair")));
  mrb_data_init(backend_pipe, mrb_zmq_thread_data->backend, &mrb_zmq_socket_type);
  mrb_value timeo = mrb_int_value(mrb, 120000);
  mrb_funcall(mrb, backend_pipe, "sndtimeo=", 1, timeo);
  mrb_funcall(mrb, backend_pipe, "rcvtimeo=", 1, timeo);

  mrb_value argv = mrb_msgpack_unpack(mrb, mrb_zmq_thread_data->argv_str);
  mrb_value block = mrb_msgpack_unpack(mrb, mrb_zmq_thread_data->block_str);
  if (mrb_type(mrb_ary_ref(mrb, argv, 0)) == MRB_TT_CLASS) {
    mrb_value bg_class = mrb_ary_shift(mrb, argv);
    enum mrb_vtype ttype = MRB_INSTANCE_TT(mrb_class_ptr(bg_class));
    if (ttype == 0) ttype = MRB_TT_OBJECT;
    mrb_zmq_thread_data->thread_fn = mrb_obj_value(mrb_obj_alloc(mrb, ttype, mrb_class_ptr(bg_class)));
  } else {
    mrb_zmq_thread_data->thread_fn = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_OBJECT, mrb_class_get_under(mrb, mrb_class_get_under(mrb, zmq_mod, "Thread"), "Thread_fn")));
  }
  mrb_iv_set(mrb, mrb_zmq_thread_data->thread_fn, mrb_intern_lit(mrb, "@pipe"), backend_pipe);
  mrb_funcall_with_block(mrb, mrb_zmq_thread_data->thread_fn, mrb_intern_lit(mrb, "initialize"), RARRAY_LEN(argv), RARRAY_PTR(argv), block);
  int success = TRUE;
  int rc = zmq_send(mrb_zmq_thread_data->backend, &success, sizeof(success), 0);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_send");
  }
}

static int
mrb_zmq_thread_fn(void *mrb_zmq_thread_data_)
{
  mrb_zmq_thread_data_t *mrb_zmq_thread_data = (mrb_zmq_thread_data_t *) mrb_zmq_thread_data_;
  int success = FALSE;
  int rc = zmq_connect(mrb_zmq_thread_data->backend, mrb_zmq_thread_data->endpoint);
  assert(rc != -1);

  mrb_state *mrb = mrb_open_allocf(mrb_zmq_thread_data->mrb_parent->allocf, mrb_zmq_thread_data->mrb_parent->allocf_ud);
  if (likely(mrb)) {
    mrb_zmq_thread_data->backend_ctx = MRB_LIBZMQ_CONTEXT(mrb);
    mrb_zmq_thread_data->thread_fn = mrb_nil_value();

    mrb_bool error = FALSE;
    mrb_value exc = mrb_protect(mrb, mrb_zmq_thread_fn_cb, mrb_cptr_value(mrb, mrb_zmq_thread_data), &error);
    success = !error;
    if (unlikely(error)) {
      if (mrb_test(exc) && mrb_exception_p(exc)) {
        exc = mrb_msgpack_pack(mrb, exc);
        if (mrb_string_p(exc)) {
          zmq_send(mrb_zmq_thread_data->backend, &success, sizeof(success), ZMQ_SNDMORE);
          zmq_send(mrb_zmq_thread_data->backend, RSTRING_PTR(exc), RSTRING_LEN(exc), 0);
        } else {
          zmq_send(mrb_zmq_thread_data->backend, &success, sizeof(success), 0);
        }
      } else {
        zmq_send(mrb_zmq_thread_data->backend, &success, sizeof(success), 0);
      }
      zmq_close(mrb_zmq_thread_data->backend);
    }

    mrb_gc_arena_restore(mrb, 0);
    if (likely(success)) {
      mrb_gc_register(mrb, mrb_zmq_thread_data->thread_fn);
      mrb_funcall(mrb, mrb_zmq_thread_data->thread_fn, "run", 0);
    }

    if (mrb->exc && !mrb_obj_is_kind_of(mrb, mrb_obj_value(mrb->exc), E_ETERM_ERROR)) {
      success = FALSE;
      mrb_print_error(mrb);
    }

    mrb_close(mrb);
  } else {
    zmq_send(mrb_zmq_thread_data->backend, &success, sizeof(success), 0);
    zmq_close(mrb_zmq_thread_data->backend);
  }

  return success;
}

static mrb_value
mrb_zmq_threadstart(mrb_state *mrb, mrb_value thread_class)
{
  if (unlikely(MRB_INSTANCE_TT(mrb_class_ptr(thread_class)) != MRB_TT_DATA)) {
    mrb_raisef(mrb, E_TYPE_ERROR, "%S instance_tt is not MRB_TT_DATA", mrb_class_path(mrb, mrb_class_ptr(thread_class)));
  }

  mrb_value *argv = NULL;
  mrb_int argv_len = 0;
  mrb_value block = mrb_nil_value();
  mrb_get_args(mrb, "*&", &argv, &argv_len, &block);

  mrb_value self = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, mrb_class_ptr(thread_class)));
  int arena_index = mrb_gc_arena_save(mrb);
  mrb_zmq_thread_data_t *mrb_zmq_thread_data = (mrb_zmq_thread_data_t *) mrb_realloc(mrb, DATA_PTR(self), sizeof(*mrb_zmq_thread_data));
  memset(mrb_zmq_thread_data, 0, sizeof(*mrb_zmq_thread_data));
  mrb_data_init(self, mrb_zmq_thread_data, &mrb_zmq_thread_type);
  mrb_zmq_thread_data->mrb_parent = mrb;
  mrb_zmq_thread_data->argv_str = mrb_msgpack_pack_argv(mrb, argv, argv_len);
  mrb_zmq_thread_data->block_str = mrb_msgpack_pack(mrb, block);

  mrb_value pipe_args[] = {
    mrb_format(mrb, "inproc://mrb-zmq-thread-pipe-%S", mrb_int_value(mrb, mrb_obj_id(self))),
    mrb_true_value()
  };
  mrb_zmq_thread_data->endpoint = mrb_string_value_cstr(mrb, &pipe_args[0]);
  mrb_value frontend_pipe = mrb_obj_new(mrb, mrb_class_get_under(mrb, mrb_module_get(mrb, "ZMQ"), "Pair"), NELEMS(pipe_args), pipe_args);
  mrb_zmq_thread_data->frontend = DATA_PTR(frontend_pipe);
  mrb_value timeo = mrb_int_value(mrb, 120000);
  mrb_funcall(mrb, frontend_pipe, "sndtimeo=", 1, timeo);
  mrb_funcall(mrb, frontend_pipe, "rcvtimeo=", 1, timeo);
  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@pipe"), frontend_pipe);
  mrb_zmq_thread_data->backend = zmq_socket(MRB_LIBZMQ_CONTEXT(mrb), ZMQ_PAIR);
  if (unlikely(!mrb_zmq_thread_data->backend)) {
    mrb_zmq_handle_error(mrb, "zmq_socket");
  }

  int rc = thrd_create(&mrb_zmq_thread_data->thread, &mrb_zmq_thread_fn, mrb_zmq_thread_data);
  if (unlikely(rc != thrd_success)) {
    zmq_close(mrb_zmq_thread_data->backend);
    mrb_zmq_handle_error(mrb, "thrd_create");
  }

  int success = FALSE;
  zmq_recv(mrb_zmq_thread_data->frontend, &success, sizeof(success), 0);
  if (unlikely(!success)) {
    mrb_bool rcvmore = mrb_bool(mrb_funcall(mrb, frontend_pipe, "rcvmore?", 0));
    if (rcvmore) {
      mrb_exc_raise(mrb, mrb_msgpack_unpack(mrb, mrb_funcall(mrb, mrb_funcall(mrb, frontend_pipe, "recv", 0), "to_str", 0)));
    } else {
      mrb_raise(mrb, E_RUNTIME_ERROR, "Cannot initialize ZMQ Thread");
    }
  }

  mrb_funcall_argv(mrb, self, mrb_intern_lit(mrb, "initialize"), argv_len, argv);
  mrb_gc_arena_restore(mrb, arena_index);

  return self;
}

static mrb_value
mrb_zmq_threadclose(mrb_state *mrb, mrb_value self)
{
  mrb_value thread_val;
  mrb_get_args(mrb, "o", &thread_val);

  if (mrb_type(thread_val) == MRB_TT_DATA && DATA_TYPE(thread_val) == &mrb_zmq_thread_type) {
    mrb_zmq_thread_data_t *mrb_zmq_thread_data = (mrb_zmq_thread_data_t *) DATA_PTR(thread_val);
    zmq_send_const(mrb_zmq_thread_data->frontend, "TERM$", 5, 0);
    int success = FALSE;
    thrd_join(mrb_zmq_thread_data->thread, &success);
    mrb_free(mrb, mrb_zmq_thread_data);
    mrb_data_init(thread_val, NULL, NULL);
    mrb_iv_remove(mrb, thread_val, mrb_intern_lit(mrb, "@pipe"));
    return mrb_bool_value(success);
  }

  return mrb_nil_value();
}

static mrb_value
mrb_zmq_threadclose_mark(mrb_state *mrb, mrb_value self)
{
  mrb_value thread_val;
  mrb_get_args(mrb, "o", &thread_val);

  if (mrb_type(thread_val) == MRB_TT_DATA && DATA_TYPE(thread_val) == &mrb_zmq_thread_type) {
    mrb_zmq_thread_data_t *mrb_zmq_thread_data = (mrb_zmq_thread_data_t *) DATA_PTR(thread_val);
    int timeo = 0;
    zmq_setsockopt(mrb_zmq_thread_data->frontend, ZMQ_SNDTIMEO, &timeo, sizeof(timeo));
    zmq_send_const(mrb_zmq_thread_data->frontend, "TERM$", 5, 0);
    zmq_ctx_shutdown(mrb_zmq_thread_data->backend_ctx);
    int success = FALSE;
    thrd_join(mrb_zmq_thread_data->thread, &success);
    mrb_free(mrb, mrb_zmq_thread_data);
    mrb_data_init(thread_val, NULL, NULL);
    mrb_iv_remove(mrb, thread_val, mrb_intern_lit(mrb, "@pipe"));
    return mrb_bool_value(success);
  }

  return mrb_nil_value();
}

MRB_INLINE void *
mrb_zmq_get_socket(mrb_state *mrb, mrb_value socket)
{
  switch(mrb_type(socket)) {
    case MRB_TT_CPTR: {
      // No way to check here if its a legitmate zmq socket, if something else is passed libzmq asserts and aborts the programm.
      // Also: when handed a raw c pointer only allow the poller to handle it because we cannot know from which thread this socket comes.
      return mrb_cptr(socket);
    }
    case MRB_TT_DATA: {
      if (DATA_TYPE(socket) == &mrb_zmq_socket_type)
        return DATA_PTR(socket);
    }
    default: {
      mrb_raise(mrb, E_TYPE_ERROR, "Expected a ZMQ Socket");
    }
  }
}

#ifdef ZMQ_HAVE_POLLER
static mrb_value
mrb_zmq_poller_new(mrb_state *mrb, mrb_value self)
{
  if (unlikely(DATA_PTR(self))) {
    mrb_free(mrb, DATA_PTR(self));
    mrb_data_init(self, NULL, NULL);
  }

  mrb_data_init(self, zmq_poller_new(), &mrb_zmq_poller_type);
  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "sockets"), mrb_ary_new(mrb));

  return self;
}

static mrb_value
mrb_zmq_poller_add(mrb_state *mrb, mrb_value self)
{
  mrb_value socket;
  mrb_int events = ZMQ_POLLIN;
  mrb_get_args(mrb, "o|i", &socket, &events);
  struct RClass *socket_class = mrb_obj_class(mrb, socket);
  mrb_assert_int_fit(mrb_int, events, short, SHRT_MAX);

  int rc;
  if (mrb_obj_respond_to(mrb, socket_class, mrb_intern_lit(mrb, "to_i"))) {
    mrb_int fd = mrb_integer(mrb_convert_to_integer(mrb, socket, 0));
    mrb_assert_int_fit(mrb_int, fd, int, INT_MAX);
    rc = zmq_poller_add_fd(DATA_PTR(self), fd, mrb_ptr(socket), events);
    if (unlikely(rc == -1)) {
      mrb_zmq_handle_error(mrb, "zmq_poller_add_fd");
    }
  } else if (mrb_obj_respond_to(mrb, socket_class, mrb_intern_lit(mrb, "zmq_socket"))) {
    rc = zmq_poller_add(DATA_PTR(self), mrb_zmq_get_socket(mrb, mrb_funcall(mrb, socket, "zmq_socket", 0)), mrb_ptr(socket), events);
  } else {
    rc = zmq_poller_add(DATA_PTR(self), mrb_zmq_get_socket(mrb, socket), mrb_ptr(socket), events);
  }

  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_poller_add");
  }

  mrb_ary_push(mrb, mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "sockets")), socket);

  return self;
}

static mrb_value
mrb_zmq_poller_modify(mrb_state *mrb, mrb_value self)
{
  mrb_value socket;
  mrb_int events;
  mrb_get_args(mrb, "oi", &socket, &events);
  struct RClass *socket_class = mrb_obj_class(mrb, socket);
  mrb_assert_int_fit(mrb_int, events, short, SHRT_MAX);

  int rc;
  if (mrb_obj_respond_to(mrb, socket_class, mrb_intern_lit(mrb, "to_i"))) {
    mrb_int fd = mrb_integer(mrb_convert_to_integer(mrb, socket, 0));
    mrb_assert_int_fit(mrb_int, fd, int, INT_MAX);
    rc = zmq_poller_modify_fd(DATA_PTR(self), fd, events);
    if (unlikely(rc == -1)) {
      mrb_zmq_handle_error(mrb, "zmq_poller_modify_fd");
    }
  } else if (mrb_obj_respond_to(mrb, socket_class, mrb_intern_lit(mrb, "zmq_socket"))) {
    rc = zmq_poller_modify(DATA_PTR(self), mrb_zmq_get_socket(mrb, mrb_funcall(mrb, socket, "zmq_socket", 0)), events);
  } else {
    rc = zmq_poller_modify(DATA_PTR(self), mrb_zmq_get_socket(mrb, socket), events);
  }

  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_poller_modify");
  }

  return self;
}

static mrb_value
mrb_zmq_poller_remove(mrb_state *mrb, mrb_value self)
{
  mrb_value socket;
  mrb_get_args(mrb, "o", &socket);
  struct RClass *socket_class = mrb_obj_class(mrb, socket);

  int rc;
  if (mrb_obj_respond_to(mrb, socket_class, mrb_intern_lit(mrb, "to_i"))) {
    mrb_int fd = mrb_integer(mrb_convert_to_integer(mrb, socket, 0));
    mrb_assert_int_fit(mrb_int, fd, int, INT_MAX);
    rc = zmq_poller_remove_fd(DATA_PTR(self), fd);
    if (unlikely(rc == -1)) {
      mrb_zmq_handle_error(mrb, "zmq_poller_remove_fd");
    }
  } else if (mrb_obj_respond_to(mrb, socket_class, mrb_intern_lit(mrb, "zmq_socket"))) {
    rc = zmq_poller_remove(DATA_PTR(self), mrb_zmq_get_socket(mrb, mrb_funcall(mrb, socket, "zmq_socket", 0)));
  } else {
    rc = zmq_poller_remove(DATA_PTR(self), mrb_zmq_get_socket(mrb, socket));
  }

  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_poller_remove");
  }

  mrb_funcall(mrb, mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "sockets")), "delete", 1, socket);

  return self;
}

static mrb_value
mrb_zmq_poller_wait(mrb_state *mrb, mrb_value self)
{
  mrb_int timeout = -1;
  mrb_value block = mrb_nil_value();
  mrb_get_args(mrb, "|i&", &timeout, &block);

  int rc;
  if (mrb_type(block) != MRB_TT_PROC) {
    zmq_poller_event_t event;
    rc = zmq_poller_wait(DATA_PTR(self), &event, (long) timeout);
    if (rc == -1) {
      switch(mrb_zmq_errno()) {
        case ETIMEDOUT: {
          return mrb_nil_value();
        }
        case EINTR: {
          return mrb_false_value();
        }
        default: {
          mrb_zmq_handle_error(mrb, "zmq_poller_wait");
        }
      }
    }

    return mrb_obj_value(event.user_data);
  } else {
    mrb_int n_events = RARRAY_LEN(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "sockets")));
    if (n_events > 0) {
      zmq_poller_event_t events[n_events];
      rc = zmq_poller_wait_all(DATA_PTR(self), events, n_events, timeout);
      int i;
      for (i = 0; i < rc; i++) {
        mrb_value argv[] = {
          mrb_obj_value(events[i].user_data),
          mrb_int_value(mrb, events[i].events)
        };
        mrb_yield_argv(mrb, block, NELEMS(argv), argv);
      }
    } else {
      zmq_poller_event_t event;
      rc = zmq_poller_wait_all(DATA_PTR(self), &event, 0, timeout);
    }

    if (rc == -1) {
      switch(mrb_zmq_errno()) {
        case ETIMEDOUT: {
          return mrb_nil_value();
        }
        case EINTR: {
          return mrb_false_value();
        }
        default: {
          mrb_zmq_handle_error(mrb, "zmq_poller_wait_all");
        }
      }
    }

    return self;
  }
}

#else

static mrb_value
mrb_zmq_poller_new(mrb_state *mrb, mrb_value self)
{
  if (unlikely(DATA_PTR(self))) {
    mrb_free(mrb, DATA_PTR(self));
  }

  mrb_data_init(self, NULL, NULL);

  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "sockets"), mrb_ary_new_capa(mrb, 1));

  return self;
}

static mrb_value
mrb_zmq_poller_add(mrb_state *mrb, mrb_value self)
{
  mrb_value socket;
  mrb_int events = ZMQ_POLLIN;

  mrb_get_args(mrb, "o|i", &socket, &events);
  mrb_assert_int_fit(mrb_int, events, short, SHRT_MAX);

  struct RClass *socket_class = mrb_obj_class(mrb, socket);
  mrb_value sockets = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "sockets"));
  struct zmq_pollitem_t *pollitems, pollitem;

  if (mrb_obj_respond_to(mrb, socket_class, mrb_intern_lit(mrb, "to_i"))) {
    mrb_int fd = mrb_integer(mrb_convert_to_integer(mrb, socket, 0));
    mrb_assert_int_fit(mrb_int, fd, int, INT_MAX);
    pollitem.socket = NULL;
    pollitem.fd = (int) fd;
  } else if (mrb_obj_respond_to(mrb, socket_class, mrb_intern_lit(mrb, "zmq_socket"))) {
    pollitem.socket = mrb_zmq_get_socket(mrb, mrb_funcall(mrb, socket, "zmq_socket", 0));
    pollitem.fd = 0;
  } else {
    pollitem.socket = mrb_zmq_get_socket(mrb, socket);
    pollitem.fd = 0;
  }

  pollitem.events = (short) events;
  pollitem.revents = 0;

  if (likely(RARRAY_LEN(sockets) + 1 > 0 &&
      RARRAY_LEN(sockets) + 1 <= SIZE_MAX / sizeof(struct zmq_pollitem_t))) {

    pollitems = (struct zmq_pollitem_t *) mrb_realloc(mrb, DATA_PTR(self), (RARRAY_LEN(sockets) + 1) * sizeof(*pollitems));
    mrb_data_init(self, pollitems, &mrb_zmq_poller_type);
    pollitems[RARRAY_LEN(sockets)] = pollitem;
    mrb_ary_push(mrb, sockets, socket);

    return self;
  } else {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "cannot add more sockets");
    return mrb_false_value();
  }
}

static mrb_value
mrb_zmq_poller_modify(mrb_state *mrb, mrb_value self)
{
  mrb_value socket;
  mrb_int events;
  mrb_get_args(mrb, "oi", &socket, &events);
  mrb_assert_int_fit(mrb_int, events, short, SHRT_MAX);

  mrb_value sockets = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "sockets"));
  struct zmq_pollitem_t *pollitems = (struct zmq_pollitem_t *) DATA_PTR(self);
  mrb_int socket_obj_id = mrb_obj_id(socket);

  for (mrb_int i = 0; i < RARRAY_LEN(sockets); i++) {
    if (socket_obj_id == mrb_obj_id(mrb_ary_ref(mrb, sockets, i))) {
      pollitems[i].events = (short) events;
      break;
    }
  }

  return self;
}

static mrb_value
mrb_zmq_poller_remove(mrb_state *mrb, mrb_value self)
{
  if (unlikely(!DATA_PTR(self))) {
    return mrb_false_value();
  }

  mrb_value socket;

  mrb_get_args(mrb, "o", &socket);

  mrb_value sockets = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "sockets"));
  struct zmq_pollitem_t *pollitems = (struct zmq_pollitem_t *) DATA_PTR(self);
  mrb_int socket_obj_id = mrb_obj_id(socket);

  if (RARRAY_LEN(sockets) == 1 && socket_obj_id == mrb_obj_id(mrb_ary_ref(mrb, sockets, 0))) {
    mrb_ary_clear(mrb, sockets);
    mrb_free(mrb, DATA_PTR(self));
    mrb_data_init(self, NULL, NULL);

    return self;
  } else {
    for (mrb_int i = 0; i < RARRAY_LEN(sockets); i++) {
      if (socket_obj_id == mrb_obj_id(mrb_ary_ref(mrb, sockets, i))) {
        struct zmq_pollitem_t *ptr = pollitems + i;
        mrb_int len = RARRAY_LEN(sockets) - i;
        while (--len) {
          *ptr = *(ptr+1);
          ++ptr;
        }
        mrb_funcall(mrb, sockets, "delete_at", 1, mrb_int_value(mrb, i));
        DATA_PTR(self) = (struct zmq_pollitem_t *) mrb_realloc(mrb, DATA_PTR(self), RARRAY_LEN(sockets) * sizeof(*pollitems));

        return self;
      }
    }
  }

  return mrb_nil_value();
}

static mrb_value
mrb_zmq_poller_wait(mrb_state *mrb, mrb_value self)
{
  mrb_int timeout = -1;
  mrb_value block = mrb_nil_value();

  mrb_get_args(mrb, "|i&", &timeout, &block);

  mrb_assert(timeout >= INT_MIN&&timeout <= INT_MAX);

  mrb_value sockets = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "sockets"));
  struct zmq_pollitem_t *pollitems = (struct zmq_pollitem_t *) DATA_PTR(self);

  errno = 0;
  int ret = zmq_poll(pollitems, RARRAY_LEN(sockets), (int) timeout);

  if (ret > 0) {
    if (mrb_type(block) == MRB_TT_PROC) {
      for (mrb_int i = 0; i < RARRAY_LEN(sockets); i++) {
        if (pollitems[i].revents) {
          mrb_value argv[] = {
            mrb_ary_ref(mrb, sockets, i),
            mrb_int_value(mrb, pollitems[i].revents)
          };
          mrb_yield_argv(mrb, block, NELEMS(argv), argv);
        }
      }
    } else {
      for (mrb_int i = 0; i < RARRAY_LEN(sockets); i++) {
        if (pollitems[i].revents) {
          return mrb_ary_ref(mrb, sockets, i);
        }
      }
    }
  }
  else if (ret == 0) {
    return mrb_nil_value();
  }
  else if (unlikely(ret == -1)) {
    if (errno == EINTR) {
      return mrb_false_value();
    } else {
      mrb_zmq_handle_error(mrb, "zmq_poll");
    }
  }
  else {
    mrb_raisef(mrb, E_RUNTIME_ERROR, "zmq_poll returned erroneous value %S", mrb_int_value(mrb, ret));
  }

  return self;
}


#endif // ZMQ_HAVE_POLLER

#ifdef ZMQ_HAVE_TIMERS
MRB_INLINE void
mrb_zmq_timer_fn(int timer_id, void *arg)
{
  mrb_yield(((mrb_zmq_timers_fn_t *) arg)->mrb, ((mrb_zmq_timers_fn_t *) arg)->block, mrb_int_value(((mrb_zmq_timers_fn_t *) arg)->mrb, ((mrb_zmq_timers_fn_t *) arg)->timer_id));
}

static mrb_value
mrb_zmq_timers_new(mrb_state *mrb, mrb_value self)
{
  if (unlikely(DATA_PTR(self))) {
    mrb_free(mrb, DATA_PTR(self));
    mrb_data_init(self, NULL, NULL);
  }

  mrb_data_init(self, zmq_timers_new(), &mrb_zmq_timers_type);
  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "timers"), mrb_hash_new(mrb));

  return self;
}

static mrb_value
mrb_zmq_timers_add(mrb_state *mrb, mrb_value self)
{
  mrb_int interval;
  mrb_value block = mrb_nil_value();
  mrb_get_args(mrb, "i&", &interval, &block);
  mrb_assert_int_fit(mrb_int, interval, size_t, SIZE_MAX);
  if (unlikely(mrb_type(block) != MRB_TT_PROC)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "no block given");
  }

  mrb_value timer = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, mrb_class_get_under(mrb, mrb_obj_class(mrb, self), "Timer")));
  mrb_zmq_timers_fn_t *timer_fn_arg = (mrb_zmq_timers_fn_t *) mrb_realloc(mrb, DATA_PTR(timer), sizeof(*timer_fn_arg));
  mrb_data_init(timer, timer_fn_arg, &mrb_zmq_timers_fn_type);
  timer_fn_arg->mrb = mrb;
  timer_fn_arg->timers = self;
  timer_fn_arg->block = block;
  mrb_iv_set(mrb, timer, mrb_intern_lit(mrb, "block"), block);

  int timer_id = zmq_timers_add(DATA_PTR(self), (size_t) interval, mrb_zmq_timer_fn, timer_fn_arg);
  if (unlikely(timer_id == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_timers_add");
  }
  timer_fn_arg->timer_id = timer_id;
  mrb_hash_set(mrb, mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "timers")), mrb_int_value(mrb, timer_id), timer);

  return timer;
}

static mrb_value
mrb_zmq_timers_set_interval(mrb_state *mrb, mrb_value self)
{
  mrb_int interval;
  mrb_get_args(mrb, "i", &interval);
  mrb_assert_int_fit(mrb_int, interval, size_t, SIZE_MAX);

  mrb_zmq_timers_fn_t *timer_fn_arg = (mrb_zmq_timers_fn_t *) DATA_PTR(self);
  int rc = zmq_timers_set_interval(DATA_PTR(timer_fn_arg->timers), timer_fn_arg->timer_id, (size_t) interval);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_timers_set_interval");
  }

  return self;
}

static mrb_value
mrb_zmq_timers_reset(mrb_state *mrb, mrb_value self)
{
  mrb_zmq_timers_fn_t *timer_fn_arg = (mrb_zmq_timers_fn_t *) DATA_PTR(self);
  int rc = zmq_timers_reset(DATA_PTR(timer_fn_arg->timers), timer_fn_arg->timer_id);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_timers_reset");
  }

  return self;
}

static mrb_value
mrb_zmq_timers_cancel(mrb_state *mrb, mrb_value self)
{
  if (DATA_PTR(self)) {
    mrb_zmq_timers_fn_t *timer_fn_arg = (mrb_zmq_timers_fn_t *) DATA_PTR(self);
    int rc = zmq_timers_cancel(DATA_PTR(timer_fn_arg->timers), timer_fn_arg->timer_id);
    if (unlikely(rc == -1)) {
      mrb_zmq_handle_error(mrb, "zmq_timers_cancel");
    }
    mrb_hash_delete_key(mrb, mrb_iv_get(mrb, timer_fn_arg->timers, mrb_intern_lit(mrb, "timers")), mrb_int_value(mrb, timer_fn_arg->timer_id));
    mrb_iv_remove(mrb, self, mrb_intern_lit(mrb, "block"));
    mrb_free(mrb, timer_fn_arg);
    mrb_data_init(self, NULL, NULL);
  }

  return mrb_nil_value();
}

static mrb_value
mrb_zmq_timers_timeout(mrb_state *mrb, mrb_value self)
{
  return mrb_int_value(mrb, zmq_timers_timeout(DATA_PTR(self)));
}

static mrb_value
mrb_zmq_timers_execute(mrb_state *mrb, mrb_value self)
{
  int rc = zmq_timers_execute(DATA_PTR(self));
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_timers_execute");
  }

  return self;
}
#endif //ZMQ_HAVE_TIMERS

#ifdef HAVE_IFADDRS_H
MRB_INLINE mrb_bool
s_valid_flags (unsigned int flags)
{
    return (flags & IFF_UP)             //  Only use interfaces that are running
           && !(flags & IFF_LOOPBACK)   //  Ignore loopback interface
           && (flags & IFF_MULTICAST)
#   if defined (IFF_SLAVE)
           && !(flags & IFF_SLAVE)      //  Ignore devices that are bonding slaves.
#   endif
           && !(flags & IFF_POINTOPOINT); //  Ignore point to point interfaces.
}

static mrb_value
mrb_network_interfaces(mrb_state *mrb, mrb_value self)
{
  const int is_ipv6 = zmq_ctx_get(MRB_LIBZMQ_CONTEXT(mrb), ZMQ_IPV6);
  if (unlikely(is_ipv6 == -1)) {
    mrb_sys_fail(mrb, "zmq_ctx_get");
  }

  mrb_value interfaces_ary = mrb_ary_new(mrb);
  struct ifaddrs *interfaces = NULL;

  struct mrb_jmpbuf* prev_jmp = mrb->jmp;
  struct mrb_jmpbuf c_jmp;

  MRB_TRY(&c_jmp)
  {
    mrb->jmp = &c_jmp;
    if (likely(getifaddrs(&interfaces) == 0)) {
      struct ifaddrs *interface;
      for (interface = interfaces; interface; interface = interface->ifa_next) {
        if (s_valid_flags(interface->ifa_flags)
          && interface->ifa_addr
          && interface->ifa_addr->sa_family == (is_ipv6 ? AF_INET6 : AF_INET)) {
          mrb_ary_push(mrb, interfaces_ary, mrb_str_new_cstr(mrb, interface->ifa_name));
        }
      }
      freeifaddrs(interfaces);
    } else {
      mrb_sys_fail(mrb, "getifaddrs");
    }
    mrb->jmp = prev_jmp;
  }
  MRB_CATCH(&c_jmp)
  {
    mrb->jmp = prev_jmp;
    freeifaddrs(interfaces);
    MRB_THROW(mrb->jmp);
  }
  MRB_END_EXC(&c_jmp);

  return interfaces_ary;
}
#endif //HAVE_IFADDRS_H

static mrb_value
mrb_zmq_pack_symbol(mrb_state *mrb, mrb_value self)
{
  mrb_sym symbol;
  mrb_get_args(mrb, "n", &symbol);

  return mrb_sym2str(mrb, symbol);
}

static mrb_value
mrb_zmq_unpack_symbol(mrb_state *mrb, mrb_value self)
{
  mrb_value string;
  mrb_get_args(mrb, "S", &string);

  return mrb_symbol_value(mrb_intern_str(mrb, string));
}

static mrb_value
mrb_zmq_pack_class(mrb_state *mrb, mrb_value self)
{
  mrb_value mrb_zmq_class;
  mrb_get_args(mrb, "C", &mrb_zmq_class);

  return mrb_class_path(mrb, mrb_class_ptr(mrb_zmq_class));
}

static mrb_value
mrb_zmq_unpack_class(mrb_state *mrb, mrb_value self)
{
  mrb_value mrb_zmq_class;
  mrb_get_args(mrb, "S", &mrb_zmq_class);

  return mrb_funcall(mrb, mrb_zmq_class, "constantize", 0);
}

static mrb_value
mrb_zmq_pack_proc(mrb_state *mrb, mrb_value self)
{
  mrb_value proc;
  mrb_get_args(mrb, "o", &proc);

  return mrb_funcall(mrb, proc, "to_irep", 0);
}

static mrb_value
mrb_zmq_unpack_proc(mrb_state *mrb, mrb_value self)
{
  mrb_value irep;
  mrb_get_args(mrb, "S", &irep);

  return mrb_funcall(mrb, mrb_obj_value(mrb->proc_class), "from_irep", 1, irep);
}

void
mrb_mruby_zmq_gem_init(mrb_state* mrb)
{
  if (sizeof(mrb_int) < sizeof(intptr_t)) {
    mrb_warn(mrb, "mruby-zmq: mrb_int is smaller than the pointer size of your System, use ZMQ::Thread at your own risk. Compile mruby with MRB_INT%S, setable in include/mrbconf.h in your mruby dir, to suppress this warning.\n", mrb_int_value(mrb, sizeof(intptr_t) * 8));
  }
  void *context = zmq_ctx_new();
  if (unlikely(!context)) {
    mrb_sys_fail(mrb, "zmq_ctx_new");
  }
  if (getenv("ZMQ_IO_THREADS")) {
    zmq_ctx_set(context, ZMQ_IO_THREADS, atoi(getenv("ZMQ_IO_THREADS")));
  }
  if (getenv("ZMQ_THREAD_SCHED_POLICY")) {
    zmq_ctx_set(context, ZMQ_THREAD_SCHED_POLICY, atoi(getenv("ZMQ_THREAD_SCHED_POLICY")));
  }
  if (getenv("ZMQ_THREAD_PRIORITY")) {
    zmq_ctx_set(context, ZMQ_THREAD_PRIORITY, atoi(getenv("ZMQ_THREAD_PRIORITY")));
  }

  struct RClass *libzmq_mod, *zmq_mod, *zmq_msg_class, *zmq_socket_class, *zmq_thread_class;

  libzmq_mod = mrb_define_module(mrb, "LibZMQ");
  mrb_define_const(mrb, libzmq_mod, "_Context", mrb_cptr_value(mrb, context));
  mrb_define_module_function(mrb, libzmq_mod, "bind", mrb_zmq_bind, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, libzmq_mod, "close", mrb_zmq_close, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, libzmq_mod, "close!", mrb_zmq_close_mark, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, libzmq_mod, "connect", mrb_zmq_connect, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, libzmq_mod, "ctx_get", mrb_zmq_ctx_get, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, libzmq_mod, "ctx_set", mrb_zmq_ctx_set, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, libzmq_mod, "curve_keypair", mrb_zmq_curve_keypair, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, libzmq_mod, "disconnect", mrb_zmq_disconnect, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, libzmq_mod, "getsockopt", mrb_zmq_getsockopt, MRB_ARGS_ARG(3, 1));
  mrb_define_module_function(mrb, libzmq_mod, "msg_gets", mrb_zmq_msg_gets, MRB_ARGS_ARG(2, 1));
  mrb_define_module_function(mrb, libzmq_mod, "msg_send", mrb_zmq_msg_send, MRB_ARGS_REQ(3));
  mrb_define_module_function(mrb, libzmq_mod, "msg_size", mrb_zmq_msg_size, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, libzmq_mod, "proxy", mrb_zmq_proxy, MRB_ARGS_ARG(2, 1));
  mrb_define_module_function(mrb, libzmq_mod, "proxy_steerable", mrb_zmq_proxy_steerable, MRB_ARGS_ARG(3, 1));
  mrb_define_module_function(mrb, libzmq_mod, "send", mrb_zmq_send, MRB_ARGS_REQ(3));
  mrb_define_module_function(mrb, libzmq_mod, "setsockopt", mrb_zmq_setsockopt, MRB_ARGS_REQ(3));
  mrb_define_module_function(mrb, libzmq_mod, "socket_monitor", mrb_zmq_socket_monitor, MRB_ARGS_REQ(3));
  mrb_define_module_function(mrb, libzmq_mod, "threadclose", mrb_zmq_threadclose, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, libzmq_mod, "threadclose!", mrb_zmq_threadclose_mark, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, libzmq_mod, "unbind", mrb_zmq_unbind, MRB_ARGS_REQ(2));
#ifdef ZMQ_HAS_CAPABILITIES
  mrb_define_module_function(mrb, libzmq_mod, "has?", mrb_zmq_has, MRB_ARGS_REQ(1));
#endif
#ifdef ZMQ_SERVER
  mrb_define_module_function(mrb, libzmq_mod, "msg_routing_id", mrb_zmq_msg_routing_id, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, libzmq_mod, "msg_set_routing_id", mrb_zmq_msg_set_routing_id, MRB_ARGS_REQ(2));
#endif
#ifdef ZMQ_DISH
  mrb_define_module_function(mrb, libzmq_mod, "join", mrb_zmq_join, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, libzmq_mod, "leave", mrb_zmq_leave, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, libzmq_mod, "msg_group", mrb_zmq_msg_group, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, libzmq_mod, "msg_set_group", mrb_zmq_msg_set_group, MRB_ARGS_REQ(2));
#endif
  mrb_define_module_function(mrb, libzmq_mod, "z85_decode", mrb_zmq_z85_decode, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, libzmq_mod, "z85_encode", mrb_zmq_z85_encode, MRB_ARGS_REQ(1));

  zmq_mod = mrb_define_module(mrb, "ZMQ");
  zmq_msg_class = mrb_define_class_under(mrb, zmq_mod, "Msg", mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_msg_class, MRB_TT_DATA);
  mrb_define_method(mrb, zmq_msg_class, "initialize", mrb_zmq_msg_new, MRB_ARGS_OPT(1));
  mrb_define_method(mrb, zmq_msg_class, "initialize_copy", mrb_zmq_msg_copy, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, zmq_msg_class, "to_str", mrb_zmq_msg_to_str, MRB_ARGS_NONE());
  mrb_define_method(mrb, zmq_msg_class, "==", mrb_zmq_msg_eql, MRB_ARGS_REQ(1));

  zmq_socket_class = mrb_define_class_under(mrb, zmq_mod, "Socket", mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_socket_class, MRB_TT_DATA);
  mrb_define_method(mrb, zmq_socket_class, "initialize", mrb_zmq_socket, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, zmq_socket_class, "recv", mrb_zmq_socket_recv, MRB_ARGS_OPT(1));

  zmq_thread_class = mrb_define_class_under(mrb, zmq_mod, "Thread", mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_thread_class, MRB_TT_DATA);
  mrb_define_class_method(mrb, zmq_thread_class, "new", mrb_zmq_threadstart, MRB_ARGS_ANY());

  struct RClass *zmq_poller_class = mrb_define_class_under(mrb, zmq_mod, "Poller", mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_poller_class, MRB_TT_DATA);
  mrb_define_const(mrb, zmq_poller_class, "In", mrb_int_value(mrb, ZMQ_POLLIN));
  mrb_define_const(mrb, zmq_poller_class, "Out", mrb_int_value(mrb, ZMQ_POLLOUT));
  mrb_define_const(mrb, zmq_poller_class, "Err", mrb_int_value(mrb, ZMQ_POLLERR));
  mrb_define_const(mrb, zmq_poller_class, "Pri", mrb_int_value(mrb, ZMQ_POLLPRI));
  mrb_define_method(mrb, zmq_poller_class, "initialize", mrb_zmq_poller_new, MRB_ARGS_NONE());
  mrb_define_method(mrb, zmq_poller_class, "add", mrb_zmq_poller_add, MRB_ARGS_ARG(1, 1));
  mrb_define_alias(mrb, zmq_poller_class, "<<", "add");
  mrb_define_method(mrb, zmq_poller_class, "modify", mrb_zmq_poller_modify, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, zmq_poller_class, "remove", mrb_zmq_poller_remove, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, zmq_poller_class, "wait", mrb_zmq_poller_wait, (MRB_ARGS_OPT(1)|MRB_ARGS_BLOCK()));

#ifdef ZMQ_HAVE_TIMERS
  struct RClass *zmq_timers_class, *zmq_timers_timer_fn_class;
  zmq_timers_class = mrb_define_class_under(mrb, zmq_mod, "Timers", mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_timers_class, MRB_TT_DATA);
  mrb_define_method(mrb, zmq_timers_class, "initialize", mrb_zmq_timers_new, MRB_ARGS_NONE());
  mrb_define_method(mrb, zmq_timers_class, "add", mrb_zmq_timers_add, (MRB_ARGS_REQ(1)|MRB_ARGS_BLOCK()));
  mrb_define_method(mrb, zmq_timers_class, "timeout", mrb_zmq_timers_timeout, MRB_ARGS_NONE());
  mrb_define_method(mrb, zmq_timers_class, "execute", mrb_zmq_timers_execute, MRB_ARGS_NONE());
  zmq_timers_timer_fn_class = mrb_define_class_under(mrb, zmq_timers_class, "Timer", mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_timers_timer_fn_class, MRB_TT_DATA);
  mrb_define_method(mrb, zmq_timers_timer_fn_class, "interval=", mrb_zmq_timers_set_interval, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, zmq_timers_timer_fn_class, "reset", mrb_zmq_timers_reset, MRB_ARGS_NONE());
  mrb_define_method(mrb, zmq_timers_timer_fn_class, "cancel", mrb_zmq_timers_cancel, MRB_ARGS_NONE());
#endif //ZMQ_HAVE_TIMERS

#ifdef HAVE_IFADDRS_H
  mrb_define_module_function(mrb, zmq_mod, "network_interfaces", mrb_network_interfaces, MRB_ARGS_NONE());
#endif

#define mrb_zmq_define_const(ZMQ_CONST_NAME, ZMQ_CONST) \
  do { \
    mrb_define_const(mrb, libzmq_mod, ZMQ_CONST_NAME, mrb_int_value(mrb, ZMQ_CONST)); \
  } while(0)

#include "zmq_const.cstub"


  mrb_value msgpack = mrb_obj_value(mrb_module_get(mrb, "MessagePack"));
  mrb_sym register_pack_type = mrb_intern_lit(mrb, "register_pack_type");
  mrb_sym register_unpack_type = mrb_intern_lit(mrb, "register_unpack_type");

  mrb_value pack_sym_args[] = { mrb_int_value(mrb, 1), mrb_obj_value(mrb->symbol_class) };
  mrb_funcall_with_block(mrb, msgpack,
    register_pack_type, NELEMS(pack_sym_args), pack_sym_args, mrb_obj_value(mrb_closure_new_cfunc(mrb, mrb_zmq_pack_symbol, 1)));
  mrb_funcall_with_block(mrb, msgpack,
    register_unpack_type, 1, pack_sym_args, mrb_obj_value(mrb_closure_new_cfunc(mrb, mrb_zmq_unpack_symbol, 1)));

  mrb_value pack_class_args[] = { mrb_int_value(mrb, 2), mrb_obj_value(mrb->class_class) };
  mrb_funcall_with_block(mrb, msgpack,
    register_pack_type, NELEMS(pack_class_args), pack_class_args, mrb_obj_value(mrb_closure_new_cfunc(mrb, mrb_zmq_pack_class, 1)));
  mrb_funcall_with_block(mrb, msgpack,
    register_unpack_type, 1, pack_class_args, mrb_obj_value(mrb_closure_new_cfunc(mrb, mrb_zmq_unpack_class, 1)));

  mrb_value pack_module_args[] = { mrb_int_value(mrb, 4), mrb_obj_value(mrb->module_class) };
  mrb_funcall_with_block(mrb, msgpack,
    register_pack_type, NELEMS(pack_module_args), pack_module_args, mrb_obj_value(mrb_closure_new_cfunc(mrb, mrb_zmq_pack_class, 1)));
  mrb_funcall_with_block(mrb, msgpack,
    register_unpack_type, 1, pack_module_args, mrb_obj_value(mrb_closure_new_cfunc(mrb, mrb_zmq_unpack_class, 1)));

  mrb_value pack_proc_args[] = { mrb_int_value(mrb, 127), mrb_obj_value(mrb->proc_class) };
  mrb_funcall_with_block(mrb, msgpack,
    register_pack_type, NELEMS(pack_proc_args), pack_proc_args, mrb_obj_value(mrb_closure_new_cfunc(mrb, mrb_zmq_pack_proc, 1)));
  mrb_funcall_with_block(mrb, msgpack,
    register_unpack_type, 1, pack_proc_args, mrb_obj_value(mrb_closure_new_cfunc(mrb, mrb_zmq_unpack_proc, 1)));

}

void
mrb_mruby_zmq_gem_final(mrb_state* mrb)
{
  void *context = MRB_LIBZMQ_CONTEXT(mrb);
  zmq_ctx_shutdown(context);
  mrb_objspace_each_objects(mrb, mrb_zmq_zmq_close_gem_final, mrb_class_get_under(mrb, mrb_module_get(mrb, "ZMQ"), "Socket"));
  zmq_ctx_term(context);
}
