// -- -- -- -- -- --
// Created by Loony-dev � 2021
// VK: https://vk.com/loonydev
// -- -- -- -

#pragma once

#include "../main.h"

class CVector {
public:
	CVector();
	CVector(float value);
	CVector(float x, float y, float z);

	VECTOR Set(float value);
	VECTOR Set(float x, float y, float z);

	VECTOR Get();

	VECTOR Zero();

private:
	VECTOR vector;
};