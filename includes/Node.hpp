#pragma once

#include "Config.hpp"

struct Node
{
    TYPE_CHAR c = 0;
    TYPE_FREQUENCY frequency = 0;
    Node *left = nullptr;
    Node *right = nullptr;

    Node(TYPE_CHAR c, TYPE_FREQUENCY frequency)
    {
        this->c = c;
        this->frequency = frequency;
    }

    Node(TYPE_FREQUENCY frequency, Node *left, Node *right)
    {
        this->frequency = frequency;
        this->left = left;
        this->right = right;
    }

    ~Node()
    {
        delete left;
        delete right;
    }

    bool is_leaf() const
    {
        return left == nullptr && right == nullptr;
    }

    bool is_valid() const
    {
        return c != 0 && frequency > 0;
    }
};