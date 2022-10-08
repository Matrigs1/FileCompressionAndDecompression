#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QBuffer>
#include <QTextStream>

//cria arquivo e coloca nele de 0 até 999 em string
bool makeFile(QString path)
{
    QFile file(path);
    if(file.open(QIODevice::WriteOnly))
    {
        QByteArray data;
        for(int i = 0; i < 1000; i++)
        {
            data.append(QString::number(i).toUtf8() + "\r\n");
        }

        file.write(data);
        file.close(); //calls flush

        return true;
    }
    return false;
}

//ponto de parada da leitura
QByteArray getHeader()
{
    QByteArray header;
    header.append("@!~!@");
    return header;
}

//recebe o path do arquivo original e o do novo
bool compressFile(QString originalFile, QString newFile)
{
    QFile ofile(originalFile);
    QFile nfile(newFile);

    QByteArray header = getHeader();

    if(!ofile.open(QIODevice::ReadOnly)) return false;
    if(!nfile.open(QIODevice::WriteOnly)) return false;
    int size = 1024;

    //enquanto o ofile não estiver no fim
    while(!ofile.atEnd())
    {
        //vai lendo o arquivo no tamanho passado e joga no buffer.
        QByteArray buffer = ofile.read(size);
        //a cada leitura, vai fazer a compressão e jogando no compressed.
        QByteArray compressed = qCompress(buffer, 9);
        //vai jogando isso no nfile. A cada 1024 bytes é escrito um novo header.
        nfile.write(header);
        nfile.write(compressed);
    }
    ofile.close();
    nfile.close();

    qInfo() << "Finished compressing";

    return true;
}

bool decompressFile(QString compressedFile, QString newFile)
{
    QFile cfile(compressedFile);
    QFile nfile(newFile);
    QByteArray header = getHeader();
    int size = 1024;

    if(!cfile.open(QIODevice::ReadOnly)) return false;
    if(!nfile.open(QIODevice::WriteOnly)) return false;

    //garantindo que o arquivo foi criado
    QByteArray buffer = cfile.peek(size);
    if(!buffer.startsWith(header))
    {
        qCritical() << "We did not create this file!";
        cfile.close();
        nfile.close();
        return false;
    }

    //achar posições do header
    cfile.seek(header.length());

    while(!cfile.atEnd())
    {
        //vai lendo o arquivo com o tamanho passado e jogando em buffer
        buffer = cfile.peek(size);
        //vai pegando o index de header
        qint64 index = buffer.indexOf(header);
        qDebug() << "Head found at: " << index;

        if(index > -1)
        {
            //achou um header
            qint64 maxbytes = index;
            qInfo() << "Reading: " << maxbytes;
            buffer = cfile.read(maxbytes);
            cfile.read(header.length());
        }
        else
        {
            //sem header
            qInfo() << "Read all, no header!";
            buffer = cfile.readAll();
        }

        QByteArray decompressed = qUncompress(buffer);
        nfile.write(decompressed);
        nfile.flush();
    }

    cfile.close();
    nfile.close();

    return true;
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString originalFile = "original.txt";
    QString compressedFile = "compressed.txt";
    QString decompressedFile = "decompressed.txt";

    if(makeFile(originalFile))
    {
        qInfo() << "Original created!";

        if(compressFile(originalFile, compressedFile))
        {
            qInfo() << "File compressed!";
            if(decompressFile(compressedFile, decompressedFile))
            {
               qInfo() << "File decompressed!";
            }
            else
            {
                qInfo() << "Could not decompress file!";
            }
        }
        else
        {
            qInfo() << "File not compressed!";
        }
    }

    return a.exec();
}
