"""Unit tests for PIPER preprocessing operators"""

import pytest
import torch


class TestStatelessOperators:
    """Test stateless preprocessing operators"""

    def test_neg2zero(self):
        """Test Neg2Zero operator"""
        input_data = torch.tensor([-2.0, -1.0, 0.0, 1.0, 2.0])
        expected = torch.tensor([0.0, 0.0, 0.0, 1.0, .0])
        result = torch.clamp(input_data, min=0.0)
        assert torch.allclose(result, expected)

    def test_logarithm(self):
        """Test Logarithm operator"""
        input_data = torch.tensor([1.0, 2.0, 10.0, 100.0])
        result = torch.log(input_data)
        assert result[0] == pytest.approx(0.0)
        assert result[1] == pytest.approx(0.693, abs=0.001)

    def test_modulus(self):
        """Test Modulus operator"""
        input_data = torch.tensor([0, 5, 10, 15, 99, 100, 101])
        result = input_data % 10
        expected = torch.tensor([0, 5, 0, 5, 9, 0, 1])
        assert torch.equal(result, expected)


class TestStatefulOperators:
    """Test stateful preprocessing operators"""

    def test_vocabmap(self):
        """Test VocabMap operator"""
        import torch.nn as nn
        vocab_size = 1000
        embedding_dim = 128
        embedding = nn.Embedding(vocab_size, embedding_dim)

        indices = torch.randint(0, vocab_size, (4, 10))
        result = embedding(indices)

        assert result.shape == (4, 10, embedding_dim)
        assert result.dtype == torch.float32
