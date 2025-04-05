#include "CRQ_Commands.h"
#include <GLES2/gl2.h>

void CRQ_Commands::rqVertexBufferSelect_HOOK(unsigned int **result) {
	unsigned int buffer = *(*result)++;

	if (!buffer) {
		return glBindBuffer(GL_ARRAY_BUFFER, 0);
                  }

	glBindBuffer(GL_ARRAY_BUFFER, buffer + 8);
	*(uint32_t*)(SA_ADDR(0x617234)) = 0;
}

void CRQ_Commands::rqVertexBufferDelete_HOOK(unsigned int **result) {
	unsigned int buffer = *(*result)++;

	if (buffer) {
		*(uint32_t *)result += 4;
		glDeleteBuffers(1, reinterpret_cast<const GLuint *>(buffer + 8));

		*(uint32_t *)(buffer + 8) = 0;
	}
}
