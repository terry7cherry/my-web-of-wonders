export default (pennies) =>
    new Intl.NumberFormat('en-GB', {style: 'currency', currency: 'GBP'}).format(pennies / 100);
