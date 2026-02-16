# Two Lips - Optil.io

**Problem:** Sylwester Swat - Two Lips

## Problem Statement

A **good** path in an undirected graph G is a sequence of distinct nodes (v₀, v₁, ..., vₖ) such that for any i, j, |i − j| ≠ 1 there is no edge vᵢ → vⱼ in graph G.

You are given an undirected simple graph G and two designated vertices - source node A and target node B. Your goal is to find two good paths (P₁, P₂) that begin in A and end in B. Additionally, those paths must not have common nodes, except for A and B.

If the found result is correct, your program will get score equal to |len(P₁) − len(P₂)|, where len(P) denotes the number of nodes on path P. You should maximize this value - the greater, the better.

## Input Format

First line contains two integers, **N, M**, denoting number of nodes and edges in the graph. Second line contains two integers, **A** and **B**, ids of the source and destination nodes. Each of the next M lines contains two integers **u, v**, denoting end nodes of the edge.

## Output Format

First line of the output should contain integer k₁ - number of nodes on the first path. Second line should contain k₁ integers - ids of nodes on the found path.

Third line of the output should contain integer k₂ - number of nodes on the second path. Fourth line should contain k₂ integers - ids of nodes on the found path.

## Constraints

- 1 ≤ N ≤ 10⁴
- 1 ≤ M ≤ 5·10⁴
- Time limit: **30 seconds per test case**
- Memory limit: 1GB
- Submission frequency: 3 minutes