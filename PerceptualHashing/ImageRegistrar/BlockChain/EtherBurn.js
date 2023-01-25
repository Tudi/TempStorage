//standalone example transaction send to smart contract address to keep everything in the single file
const ethers = require("ethers");

const BSC_RPC_NODE_URL = "https://data-seed-prebsc-1-s1.binance.org:8545/";

//dummy metamask account private key, filled with some test BNB
const PRIV_KEY = "134a42d96a4dddbe504b13cfdcf9628fa3ac60aaae9ec6c264a83628213a2cc2"

const recipient = "0xBfc3F3044864c3fE0B21d4eedE357fd4Cd55eF4e";

main();

async function main() {

const connection = new ethers.providers.JsonRpcProvider(BSC_RPC_NODE_URL);
const gasPrice = connection.getGasPrice();

const sendingAccount = new ethers.Wallet(PRIV_KEY);
const signer = sendingAccount.connect(connection);

const tx = {
    from: sendingAccount.address,
    to: recipient,
    value: ethers.utils.parseUnits('0.001','ether'),
    gasPrice: gasPrice,
    gasLimit: ethers.utils.hexlify(1000000),
    nonce: connection.getTransactionCount(sendingAccount.address, 'latest')
};

const transaction = await signer.sendTransaction(tx);
console.log(transaction)

}
