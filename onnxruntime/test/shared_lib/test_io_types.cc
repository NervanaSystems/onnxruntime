// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "core/session/onnxruntime_cxx_api.h"
#include "test_fixture.h"

using namespace onnxruntime;

static void TestModelInfo(const ONNXSession* inference_session, bool is_input, const std::vector<int64_t>& dims) {
  size_t input_count;
  if (is_input) {
    ORT_THROW_ON_ERROR(OrtInferenceSessionGetInputCount(inference_session, &input_count));
  } else {
    ORT_THROW_ON_ERROR(OrtInferenceSessionGetOutputCount(inference_session, &input_count));
  }
  ASSERT_EQ(1, input_count);
  std::unique_ptr<OrtTypeInfo> input_type_info;
  {
    OrtTypeInfo* t;
    if (is_input) {
      ORT_THROW_ON_ERROR(OrtInferenceSessionGetInputTypeInfo(inference_session, 0, &t));
    } else {
      ORT_THROW_ON_ERROR(OrtInferenceSessionGetOutputTypeInfo(inference_session, 0, &t));
    }
    input_type_info.reset(t);
  }
  ASSERT_NE(nullptr, input_type_info);
  const OrtTensorTypeAndShapeInfo* p = OrtCastTypeInfoToTensorInfo(input_type_info.get());
  ASSERT_NE(nullptr, p);

  enum OrtTensorElementDataType ele_type = OrtGetTensorElementType(p);
  ASSERT_EQ(ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT, ele_type);
  ASSERT_EQ(dims.size(), OrtGetNumOfDimensions(p));
  std::vector<int64_t> real_dims(dims.size());
  OrtGetDimensions(p, real_dims.data(), real_dims.size());
  ASSERT_EQ(real_dims, dims);
}

TEST_F(CApiTest, input_output_type_info) {
  SessionOptionsWrapper sf(env);
  constexpr PATH_TYPE model_uri = TSTR("../models/opset8/test_squeezenet/model.onnx");
  std::unique_ptr<ONNXSession, decltype(&ReleaseONNXSession)> inference_session(sf.OrtCreateInferenceSession(model_uri), ReleaseONNXSession);
  TestModelInfo(inference_session.get(), true, {1, 3, 224, 224});
  TestModelInfo(inference_session.get(), false, {1, 1000, 1, 1});
}
