# pico-libs

Este repositório contém uma coleção de bibliotecas e utilitários para projetos baseados no Raspberry Pi Pico ou similares (Pico 2, e versões com WiFi).

## Use no seu projeto

Esse repositório foi pensado como um módulo extra para projetos e não tem suporte para uso no estilo **standalone**, ou seja, precisa ser configurado um ``CMakeLists.txt`` a parte para que os exemplos sejam compilados.

## Conteúdo

- Bibliotecas para periféricos (GPIO, UART, SPI, I2C), sensores e atuadores
- Exemplos de uso
- Documentação e tutoriais

## Requisitos

- Raspberry Pi Pico ou compatível
- SDK do Pico instalado

## Como usar

1. Clone este repositório:
    ```bash
    git clone https://github.com/seu-usuario/pico-libs.git
    ```
2. Importe as bibliotecas desejadas no seu projeto.
3. Siga os exemplos nas pastas `examples` de cada biblioteca.

## Contribuição

Contribuições são bem-vindas! Abra uma issue ou envie um pull request.

## Licença

Este projeto está licenciado sob a licença MIT.