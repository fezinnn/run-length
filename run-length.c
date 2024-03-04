#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estrutura para armazenar informações da imagem PGM.
typedef struct
{
    int largura, altura, maxval;
    unsigned char *dados;
} ImagemPGM;

// Função para ler imagem PGM do arquivo.
ImagemPGM *lerPGM(const char *nomeArquivo)
{
    // Abre arquivo para leitura.
    FILE *arquivo = fopen(nomeArquivo, "r");
    // Verifica se o arquivo foi aberto corretamente.
    if (arquivo == NULL)
    {
        perror("Erro ao abrir o arquivo");
        return NULL;
    }

    // Aloca memória para a estrutura da imagem.
    ImagemPGM *imagem = (ImagemPGM *)malloc(sizeof(ImagemPGM));
    // Verifica se a memória foi alocada corretamente.
    if (imagem == NULL)
    {
        perror("Erro ao alocar memória para a imagem");
        fclose(arquivo);
        return NULL;
    }

    char linha[1024];
    fgets(linha, sizeof(linha), arquivo); // Lê o tipo do arquivo PGM (P2)
    // Ignora comentários.
    do
    {
        fgets(linha, sizeof(linha), arquivo); // Lê comentários ou dimensões.
    } while (linha[0] == '#');
    // Lê largura e altura da imagem.
    sscanf(linha, "%d %d", &imagem->largura, &imagem->altura);
    // Lê o valor máximo de cor.
    fscanf(arquivo, "%d", &imagem->maxval);

    // Aloca memória para os dados da imagem (pixels).
    imagem->dados = (unsigned char *)malloc(imagem->largura * imagem->altura);
    if (imagem->dados == NULL)
    {
        perror("Erro ao alocar memória para os dados da imagem");
        free(imagem);
        fclose(arquivo);
        return NULL;
    }

    // Lê os valores dos pixels.
    for (int i = 0; i < imagem->largura * imagem->altura; i++)
    {
        int pixel;
        fscanf(arquivo, "%d", &pixel);
        imagem->dados[i] = (unsigned char)pixel;
    }

    // Fecha o arquivo.
    fclose(arquivo);
    return imagem;
}

// Função para escrever imagem PGM no arquivo.
void escreverPGM(const char *nomeArquivo, const ImagemPGM *imagem)
{
    // Abre arquivo para escrita.
    FILE *arquivo = fopen(nomeArquivo, "w");
    if (arquivo == NULL)
    {
        perror("Erro ao abrir o arquivo para escrita");
        return;
    }

    // Escreve o cabeçalho da imagem PGM.
    fprintf(arquivo, "P2\n%d %d\n%d\n", imagem->largura, imagem->altura, imagem->maxval);
    // Escreve os dados dos pixels.
    for (int i = 0; i < imagem->largura * imagem->altura; i++)
    {
        fprintf(arquivo, "%d ", imagem->dados[i]);
        if ((i + 1) % imagem->largura == 0)
            fprintf(arquivo, "\n");
    }

    // Fecha o arquivo.
    fclose(arquivo);
}

// Função para compactar imagem PGM usando RLE.
void compactarRunLength(const ImagemPGM *imagem, const char *nomeArquivo) {
    // Abre arquivo para escrita.
    FILE *arquivo = fopen(nomeArquivo, "w");
    if (!arquivo) {
        perror("Erro ao abrir arquivo para escrita");
        return;
    }

    // Escreve cabeçalho para arquivo compactado.
    fprintf(arquivo, "P8\n%d %d\n%d\n", imagem->largura, imagem->altura, imagem->maxval);

    // Processo de compactação RLE.
    for (int y = 0; y < imagem->altura; y++) {
        int x = 0;
        while (x < imagem->largura) {
            int idx = y * imagem->largura + x; // Índice do pixel atual.
            unsigned char valor = imagem->dados[idx];
            int count = 1;
            // Conta quantos pixels consecutivos têm o mesmo valor.
            while (x + count < imagem->largura && imagem->dados[idx + count] == valor) {
                count++;
            }

            // Se a sequência tem 4 ou mais elementos, compacta. Caso contrário, escreve normalmente.
            if (count >= 4) {
                fprintf(arquivo, "@ %d %d ", valor, count);
            } else {
                for (int i = 0; i < count; i++) {
                    fprintf(arquivo, "%d ", valor);
                }
            }
            x += count;
        }
        fprintf(arquivo, "\n");
    }

    // Fecha o arquivo.
    fclose(arquivo);
}

// Função para descompactar imagem PGM compactada com RLE.
void descompactarRunLength(const char* nomeArquivoEntrada, const char* nomeArquivoSaida) {
    // Abre arquivo para leitura.
    FILE* arquivoEntrada = fopen(nomeArquivoEntrada, "r");
    if (!arquivoEntrada) {
        perror("Erro ao abrir arquivo para leitura");
        return;
    }

    // Lê cabeçalho do arquivo compactado.
    char formato[3];
    int largura, altura, maxval;
    fscanf(arquivoEntrada, "%s %d %d %d ", formato, &largura, &altura, &maxval);

    // Aloca memória para imagem descompactada.
    ImagemPGM imagem;
    imagem.largura = largura;
    imagem.altura = altura;
    imagem.maxval = maxval;
    imagem.dados = (unsigned char*)malloc(largura * altura);

    int idx = 0, valor, count;
    char ch;
    // Processo de descompactação RLE.
    while (idx < largura * altura && fscanf(arquivoEntrada, " %c", &ch) != EOF) {
        if (ch == '@') { // Indica sequência compactada.
            fscanf(arquivoEntrada, " %d %d", &valor, &count);
        } else {
            ungetc(ch, arquivoEntrada);
            fscanf(arquivoEntrada, "%d", &valor);
            count = 1;
        }

        for (int i = 0; i < count && idx < largura * altura; i++) {
            imagem.dados[idx++] = valor;
        }
    }

    // Fecha arquivo de entrada.
    fclose(arquivoEntrada);

    // Abre arquivo para escrita da imagem descompactada.
    FILE* arquivoSaida = fopen(nomeArquivoSaida, "w");
    if (!arquivoSaida) {
        perror("Erro ao abrir arquivo para escrita");
        free(imagem.dados); // Libera a memória alocada para os dados da imagem em caso de falha ao abrir o arquivo.
        return;
    }

    // Escreve o cabeçalho da imagem PGM descompactada no arquivo de saída.
    fprintf(arquivoSaida, "P2\n%d %d\n%d\n", largura, altura, maxval);
    // Escreve os dados dos pixels da imagem descompactada.
    for (int i = 0; i < largura * altura; i++) {
        fprintf(arquivoSaida, "%d", imagem.dados[i]);
        if ((i + 1) % largura == 0) {
            fprintf(arquivoSaida, "\n"); // Insere uma nova linha após escrever todos os pixels de uma linha.
        } else {
            fprintf(arquivoSaida, " "); // Insere espaço entre os valores dos pixels.
        }
    }

    // Fecha o arquivo de saída e libera a memória alocada para os dados da imagem.
    fclose(arquivoSaida);
    free(imagem.dados);
}

// Função principal que determina se o programa deve compactar ou descompactar baseado na extensão do arquivo.
int main()
{
    // Caminhos dos arquivos de entrada e saída definidos diretamente no código.
    char *entrada = "C:\\Users\\ppand\\Downloads\\balloons_noisy.ascii.pgm";
    char *saida = "C:\\Users\\ppand\\Downloads\\balaoCompactado.pgmc";

    // Determina o modo com base na extensão do arquivo de entrada.
    char *extensao = strrchr(entrada, '.');
    if (extensao != NULL)
    {
        if (strcmp(extensao, ".pgm") == 0)
        {
            // Modo de compactação: lê a imagem PGM e a compacta usando RLE.
            ImagemPGM *imagem = lerPGM(entrada);
            if (imagem != NULL)
            {
                compactarRunLength(imagem, saida);
                free(imagem->dados); // Libera memória alocada para os dados da imagem.
                free(imagem); // Libera memória alocada para a estrutura da imagem.
            }
        }
        else if (strcmp(extensao, ".pgmc") == 0)
        {
            // Modo de descompactação: lê o arquivo compactado e cria uma imagem PGM descompactada.
            descompactarRunLength(entrada, saida);
        }
        else
        {
            // Erro caso a extensão do arquivo não seja suportada.
            printf("Extensão de arquivo inválida. Use '.pgm' para compactar ou '.pgmc' para descompactar.\n");
            return 1; // Encerra o programa com código de erro.
        }
    }
    else
    {
        // Erro caso não seja possível determinar a extensão do arquivo.
        printf("Não foi possível determinar o modo a partir do arquivo de entrada. Certifique-se de que a extensão seja '.pgm' ou '.pgmc'.\n");
        return 1; // Encerra o programa com código de erro.
    }

    return 0; // Encerra o programa com sucesso.
}
