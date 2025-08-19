/**
 * @file pipeline.h
 * @brief Helpers for executing N-stage pipelines.
 */
#ifndef PIPELINE_H
#define PIPELINE_H
/** \defgroup group_pipeline pipeline
 *  @brief Multi-stage pipeline execution helpers.
 *  @{ */

#include "ast.h"

/** Execute a pipeline of count commands; returns last stage status.
 * The array must contain count valid AST command nodes.
 */
int pipeline_execute(ast_node_t **commands, int count);

/** @} */

#endif // PIPELINE_H
